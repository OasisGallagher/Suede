#include <OpenThreads/ScopedLock>

#include "time2.h"
#include "worldinternal.h"
#include "debug/profiler.h"
#include "uniformbuffermanager.h"
#include "internal/file/entityimporter.h"
#include "internal/base/transforminternal.h"
#include "internal/entities/entityinternal.h"
#include "internal/geometry/geometryutility.h"
#include "internal/world/environmentinternal.h"

#define LockEventContainerInScope()	OpenThreads::ScopedLock<OpenThreads::Mutex> lock(eventContainerMutex_)

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
	, importer_(MEMORY_CREATE(EntityImporter))
	, environment_(MEMORY_CREATE(EnvironmentInternal))
	, root_(Factory::Create<EntityInternal>()), decals_(MAX_DECALS) {
	Transform transform = Factory::Create<TransformInternal>();
	root_->SetTransform(transform);

	Profiler::Initialize();
	UniformBufferManager::Initialize();
}

WorldInternal::~WorldInternal() {
	UniformBufferManager::Destroy();
	MEMORY_RELEASE(importer_);
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
	return importer_->Import(path);
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
	WorldEventType type = e->GetEventType();
	WorldEventCollection& collection = events_[type];

	LockEventContainerInScope();
	if (collection.find(e) == collection.end()) {
		collection.insert(e);
		return true;
	}

	return false;
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
	LockEventContainerInScope();

	for (uint i = 0; i < WorldEventTypeCount; ++i) {
		WorldEventCollection& collection = events_[i];
		for (WorldEventCollection::const_iterator ite = collection.begin(); ite != collection.end(); ++ite) {
			FireEventImmediate(*ite);
		}

		collection.clear();
	}
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

	uint* indexes = mesh->MapIndexes();
	glm::vec3* vertices = mesh->MapVertices();

	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		SubMesh subMesh = mesh->GetSubMesh(i);
		const TriangleBias& bias = subMesh->GetTriangleBias();

		// TODO: triangle strip.
		for (int j = 0; j < bias.indexCount; j += 3) {
			std::vector<glm::vec3> polygon;
			uint index0 = indexes[bias.baseIndex + j] + bias.baseVertex;
			uint index1 = indexes[bias.baseIndex + j + 1] + bias.baseVertex;
			uint index2 = indexes[bias.baseIndex + j + 2] + bias.baseVertex;
			
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

	mesh->UnmapIndexes();
	mesh->UnmapVertices();

	return triangles.size() >= 3;
}

void WorldInternal::Update() {
	//Debug::StartSample();
	importer_->Update();

	//Debug::StartSample();
	FireEvents();
	//Debug::Output("[events]\t%.3f\n", Debug::EndSample());
	
	Profiler::StartSample();
	UpdateEntities();
	Debug::Output("[entities]\t%.3f\n", Profiler::EndSample());
	
	Profiler::StartSample();
	UpdateDecals();
	Debug::Output("[decals]\t%.3f\n", Profiler::EndSample());
	
	Profiler::StartSample();
	RenderUpdate();
	Debug::Output("[render]\t%.3f\n", Profiler::EndSample());

	//Debug::Output("[#total]\t%.3f\n", Debug::EndSample());
}
