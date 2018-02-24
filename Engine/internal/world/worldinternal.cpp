#include "resources.h"
#include "tools/math2.h"
#include "worldinternal.h"
#include "internal/file/assetimporter.h"
#include "internal/base/transforminternal.h"
#include "internal/entities/entityinternal.h"
#include "internal/geometry/geometryutility.h"
#include "internal/world/environmentinternal.h"

World& WorldInstance() {
	static World instance = Factory::Create<WorldInternal>();
	return instance;
}

bool WorldInternal::LightComparer::operator()(const Light & lhs, const Light & rhs) const {
	// Directional light > Importance > Luminance.
	ObjectType lt = lhs->GetType(), rt = rhs->GetType();
	if (lt != rt && (lt == ObjectTypeDirectionalLight || rt == ObjectTypeDirectionalLight)) {
		return lt == ObjectTypeDirectionalLight;
	}

	LightImportance lli = lhs->GetImportance(), rli = rhs->GetImportance();
	if (lli != rli) {
		return lli > rli;
	}

	return Math::Luminance(lhs->GetColor()) > Math::Luminance(rhs->GetColor());
}

bool WorldInternal::CameraComparer::operator() (const Camera& lhs, const Camera& rhs) const {
	return lhs->GetDepth() < rhs->GetDepth();
}

bool WorldInternal::ProjectorComparer::operator() (const Projector& lhs, const Projector& rhs) const {
	return lhs->GetDepth() < rhs->GetDepth();
}

WorldInternal::WorldInternal()
	: ObjectInternal(ObjectTypeWorld)
	, environment_(MEMORY_CREATE(EnvironmentInternal))
	, root_(Factory::Create<EntityInternal>()), decals_(MAX_DECALS) {
	Transform transform = Factory::Create<TransformInternal>();
	root_->SetTransform(transform);
}

Object WorldInternal::Create(ObjectType type) {
	Object object = Factory::Create(type);
	if (type >= ObjectTypeEntity) {
		Entity entity = dsp_cast<Entity>(object);
		Transform transform = Factory::Create<TransformInternal>();
		entity->SetTransform(transform);

		EntityCreatedEventPointer e = NewWorldEvent<EntityCreatedEventPointer>();
		e->entity = entity;
		FireEvent(e);

		transform->SetParent(root_->GetTransform());
		entities_.insert(std::make_pair(entity->GetInstanceID(), entity));
	}

	if (type >= ObjectTypeSpotLight && type <= ObjectTypeDirectionalLight) {
		lights_.insert(dsp_cast<Light>(object));
	}

	if (type == ObjectTypeCamera) {
		cameras_.insert(dsp_cast<Camera>(object));
	}

	if (type == ObjectTypeProjector) {
		projectors_.insert(dsp_cast<Projector>(object));
	}

	return object;
}

Entity WorldInternal::Import(const std::string& path) {
	AssetImporter importer;
	return importer.Import(path);
}

Entity WorldInternal::GetEntity(uint id) {
	EntityContainer::iterator ite = entities_.find(id);
	if (ite == entities_.end()) { return nullptr; }
	return ite->second;
}

bool WorldInternal::GetEntities(ObjectType type, std::vector<Entity>& entities) {
	if (type < ObjectTypeEntity) {
		Debug::LogError("invalid entity type");
		return false;
	}

	if (type == ObjectTypeEntity) {
		for (EntityContainer::iterator ite = entities_.begin(); ite != entities_.end(); ++ite) {
			entities.push_back(ite->second);
		}
	}
	else if (type == ObjectTypeCamera) {
		entities.assign(cameras_.begin(), cameras_.end());
	}
	else if (type == ObjectTypeLights) {
		entities.assign(lights_.begin(), lights_.end());
	}
	else if (type == ObjectTypeProjector) {
		entities.assign(projectors_.begin(), projectors_.end());
	}
	else {
		for (EntityContainer::iterator ite = entities_.begin(); ite != entities_.end(); ++ite) {
			if (ite->second->GetType() == type) {
				entities.push_back(ite->second);
			}
		}
	}

	return !entities.empty();
}

void WorldInternal::AddEventListener(WorldEventListener* listener) {
	if (listener == nullptr) {
		Debug::LogError("invalid world event listener.");
		return;
	}

	if (std::find(listeners_.begin(), listeners_.end(), listener) == listeners_.end()) {
		listeners_.push_back(listener);
	}
}

void WorldInternal::RemoveEventListener(WorldEventListener* listener) {
	EventListenerContainer::iterator pos = std::find(listeners_.begin(), listeners_.end(), listener);
	if (pos != listeners_.end()) {
		listeners_.erase(pos);
	}
}

bool WorldInternal::FireEvent(WorldEventBasePointer e) {
	for (WorldEventContainer::const_iterator ite = events_.begin(); ite != events_.end(); ++ite) {
		if ((*ite)->Equals(e)) {
			return false;
		}
	}

	events_.push_back(e);
	return true;
}

void WorldInternal::FireEventImmediate(WorldEventBasePointer e) {
	for (int i = 0; i < listeners_.size(); ++i) {
		listeners_[i]->OnWorldEvent(e);
	}
}

void WorldInternal::GetDecals(std::vector<Decal*>& container) {
	for (DecalContainer::iterator ite = decals_.begin(); ite != decals_.end(); ++ite) {
		container.push_back(*ite);
	}
}

void WorldInternal::RenderUpdate() {
	for (CameraContainer::iterator ite = cameras_.begin(); ite != cameras_.end(); ++ite) {
		if ((*ite)->GetActive()) {
			(*ite)->Render();
		}
	}
}

void WorldInternal::UpdateDecals() {
	decals_.clear();
	CreateDecals(*cameras_.begin());
}

void WorldInternal::UpdateEntities() {
	for (EntityContainer::iterator ite = entities_.begin(); ite != entities_.end(); ++ite) {
		if (ite->second->GetActive()) {
			ite->second->Update();
		}
	}
}

void WorldInternal::FireEvents() {
	for (WorldEventContainer::const_iterator ite = events_.begin(); ite != events_.end(); ++ite) {
		FireEventImmediate(*ite);
	}

	events_.clear();
}

void WorldInternal::CreateDecals(Camera camera) {
	for (ProjectorContainer::iterator ite = projectors_.begin(); ite != projectors_.end(); ++ite) {
		Projector p = *ite;
		GeometryUtility::CalculateFrustumPlanes(planes_, p->GetProjectionMatrix() * p->GetTransform()->GetWorldToLocalMatrix());

		if (!CreateProjectorDecal(camera, p, planes_)) {
			break;
		}
	}
}

bool WorldInternal::CreateEntityDecal(Camera camera, Decal& decal, Entity entity, Plane planes[6]) {
	std::vector<glm::vec3> triangles;
	if (!ClampMesh(camera, triangles, entity, planes)) {
		return false;
	}

	std::vector<uint> indexes;
	indexes.reserve(triangles.size());
	for (uint i = 0; i < triangles.size(); ++i) {
		indexes.push_back(i);
	}

	decal.indexes = indexes;
	decal.positions = triangles;
	decal.topology = MeshTopologyTriangles;

	return true;
}

bool WorldInternal::CreateProjectorDecal(Camera camera, Projector p, Plane planes[6]) {
	for (EntityContainer::iterator ite = entities_.begin(); ite != entities_.end(); ++ite) {
		Entity entity = ite->second;
		if (entity == p) { continue; }
		if (!entity->GetMesh()) { continue; }
		
		Decal* decal = decals_.spawn();
		if (decal == nullptr) {
			Debug::LogError("too many decals");
			return false;
		}

		if (!CreateEntityDecal(camera, *decal, entity, planes)) {
			decals_.recycle(decal);
		}

		decal->texture = p->GetTexture();
		decal->matrix = p->GetProjectionMatrix() * p->GetTransform()->GetWorldToLocalMatrix();
	}

	return true;
}

bool WorldInternal::ClampMesh(Camera camera, std::vector<glm::vec3>& triangles, Entity entity, Plane planes[6]) {
	Mesh mesh = entity->GetMesh();
	glm::vec3 cameraPosition = entity->GetTransform()->InverseTransformPoint(camera->GetTransform()->GetPosition());

	const std::vector<uint>& indexes = mesh->GetIndexes();
	const std::vector<glm::vec3>& vertices = mesh->GetVertices();

	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		SubMesh subMesh = mesh->GetSubMesh(i);
		uint indexCount, baseVertex, baseIndex;
		subMesh->GetTriangles(indexCount, baseVertex, baseIndex);

		// TODO: triangle strip.
		for (int j = 0; j < indexCount; j += 3) {
			std::vector<glm::vec3> polygon;
			uint index0 = indexes[baseIndex + j] + baseVertex;
			uint index1 = indexes[baseIndex + j + 1] + baseVertex;
			uint index2 = indexes[baseIndex + j + 2] + baseVertex;
			
			glm::vec3 vs[] = { vertices[index0], vertices[index1], vertices[index2] };

			if (!GeometryUtility::IsFrontFace(vs, cameraPosition)) {
				continue;
			}

			vs[0] = entity->GetTransform()->TransformPoint(vs[0]);
			vs[1] = entity->GetTransform()->TransformPoint(vs[1]);
			vs[2] = entity->GetTransform()->TransformPoint(vs[2]);

			GeometryUtility::ClampTriangle(polygon, vs, planes, 6);
			GeometryUtility::Triangulate(triangles, polygon, glm::cross(vs[1] - vs[0], vs[2] - vs[1]));
		}
	}

	return triangles.size() >= 3;
}

void WorldInternal::Update() {
	Debug::StartSample();

	Debug::StartSample();
	FireEvents();
	Debug::Output("[events]\t%.3f\n", Debug::EndSample());

	Debug::StartSample();
	UpdateEntities();
	Debug::Output("[entities]\t%.3f\n", Debug::EndSample());
	
	Debug::StartSample();
	UpdateDecals();
	Debug::Output("[decals]\t%.3f\n", Debug::EndSample());
	
	Debug::StartSample();
	RenderUpdate();
	Debug::Output("[render]\t%.3f\n", Debug::EndSample());

	Debug::Output("[#total]\t%.3f\n", Debug::EndSample());
}
