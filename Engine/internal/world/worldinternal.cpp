#include <OpenThreads/ScopedLock>

#include "worldinternal.h"
#include "debug/profiler.h"
#include "geometryutility.h"
#include "internal/base/framebuffer.h"
#include "internal/base/renderdefines.h"
#include "internal/file/asyncentityimporter.h"
#include "internal/entities/entityinternal.h"
#include "internal/world/environmentinternal.h"
#include "internal/components/transforminternal.h"
#include "internal/rendering/uniformbuffermanager.h"

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
	, importer_(MEMORY_CREATE(AsyncEntityImporter))
	, environment_(MEMORY_CREATE(EnvironmentInternal))
	, root_(Factory::Create<EntityInternal>()), decals_(SUEDE_MAX_DECALS) {
	Transform transform = Factory::Create<TransformInternal>();
	root_->SetTransform(transform);

	Profiler::Initialize();
	UniformBufferManager::Initialize();

	update_entities = Profiler::CreateSample();
	update_decals = Profiler::CreateSample();
	update_rendering = Profiler::CreateSample();
}

WorldInternal::~WorldInternal() {
	UniformBufferManager::Destroy();
	MEMORY_RELEASE(importer_);

	Profiler::ReleaseSample(update_entities);
	Profiler::ReleaseSample(update_decals);
	Profiler::ReleaseSample(update_rendering);
}

Object WorldInternal::Create(ObjectType type) {
	Object object = Factory::Create(type);
	if (type >= ObjectTypeEntity) {
		Entity entity = suede_dynamic_cast<Entity>(object);
		Transform transform = Factory::Create<TransformInternal>();
		entity->SetTransform(transform);

		EntityCreatedEventPointer e = NewWorldEvent<EntityCreatedEventPointer>();
		e->entity = entity;
		FireEvent(e);

		transform->SetParent(root_->GetTransform());
		entities_.insert(std::make_pair(entity->GetInstanceID(), entity));
	}

	if (type >= ObjectTypeSpotLight && type <= ObjectTypeDirectionalLight) {
		lights_.insert(suede_dynamic_cast<Light>(object));
	}

	if (type == ObjectTypeCamera) {
		cameras_.insert(suede_dynamic_cast<Camera>(object));
	}

	if (type == ObjectTypeProjector) {
		projectors_.insert(suede_dynamic_cast<Projector>(object));
	}

	return object;
}

Entity WorldInternal::Import(const std::string& path, EntityImportedListener* listener) {
	importer_->SetImportedListener(listener);
	return importer_->Import(path);
}

Entity WorldInternal::GetEntity(uint id) {
	EntityDictionary::iterator ite = entities_.find(id);
	if (ite == entities_.end()) { return nullptr; }
	return ite->second;
}

bool WorldInternal::GetEntities(ObjectType type, std::vector<Entity>& entities, EntitySelector* selector) {
	if (type < ObjectTypeEntity) {
		Debug::LogError("invalid entity type");
		return false;
	}

	if (type == ObjectTypeEntity) {
		for (EntityDictionary::iterator ite = entities_.begin(); ite != entities_.end(); ++ite) {
			if (selector == nullptr || selector->Select(ite->second)) {
				entities.push_back(ite->second);
			}
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
		for (EntityDictionary::iterator ite = entities_.begin(); ite != entities_.end(); ++ite) {
			if (ite->second->GetType() == type && (selector == nullptr || selector->Select(ite->second))) {
				entities.push_back(ite->second);
			}
		}
	}

	return !entities.empty();
}

bool WorldInternal::GetVisibleEntities(std::vector<Entity>& entities, const glm::mat4& worldToClipMatrix) {
	GetRenderableEntitiesInHierarchy(entities, root_->GetTransform(), worldToClipMatrix);
	return !entities.empty();
}

void WorldInternal::GetRenderableEntitiesInHierarchy(std::vector<Entity>& entities, Transform root, const glm::mat4 & worldToClipMatrix) {
	for (int i = 0; i < root->GetChildCount(); ++i) {
		if (root->GetEntity()->GetStatus() != EntityStatusReady) {
			Debug::Break();
		}

		Entity child = root->GetChildAt(i)->GetEntity();
		if (child->GetStatus() != EntityStatusReady) {
			continue;
		}

		// TODO: fix bug for particle system by calculating its bounds.
		if (!IsVisible(child, worldToClipMatrix)) {
		//	continue;
		}

		if (child->GetActive() && child->GetRenderer() && child->GetMesh()) {
			entities.push_back(child);
		}

		GetRenderableEntitiesInHierarchy(entities, child->GetTransform(), worldToClipMatrix);
	}
}

bool WorldInternal::IsVisible(Entity entity, const glm::mat4 & worldToClipMatrix) {
	const Bounds& bounds = entity->GetBounds();
	if (bounds.IsEmpty()) {
		return false;
	}

	return FrustumCulling(bounds, worldToClipMatrix);
}

bool WorldInternal::FrustumCulling(const Bounds & bounds, const glm::mat4 & worldToClipMatrix) {
	std::vector<glm::vec3> points;
	GeometryUtility::GetCuboidCoordinates(points, bounds.center, bounds.size);

	bool inside = false;
	glm::vec3 min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::lowest());
	for (int i = 0; i < points.size(); ++i) {
		glm::vec4 p = worldToClipMatrix * glm::vec4(points[i], 1);
		p /= p.w;
		if (p.x >= -1 && p.x <= 1 && p.y >= -1 && p.y <= 1 && p.z >= -1 && p.z <= 1) {
			inside = true;
		}

		points[i] = glm::vec3(p);
		min = glm::min(min, points[i]);
		max = glm::max(max, points[i]);
	}

	if (inside) {
		glm::vec2 size(max.x - min.x, max.y - min.y);
		return glm::dot(size, size) > MIN_NDC_RADIUS_SQUARED;
	}

	return false;
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
	Framebuffer0::Get()->Clear(FramebufferClearMaskColorDepthStencil);

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
	for (EntityDictionary::iterator ite = entities_.begin(); ite != entities_.end(); ++ite) {
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
	std::vector<Entity> entities;
	if (!GetVisibleEntities(entities, p->GetProjectionMatrix() * p->GetTransform()->GetWorldToLocalMatrix())) {
		return true;
	}

	for (std::vector<Entity>::iterator ite = entities.begin(); ite != entities.end(); ++ite) {
		Entity entity = *ite;
		if (entity == p) { continue; }
		
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

		// TODO: use triangle strip?
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
	//Debug::StartSample();
	FireEvents();
	//Debug::Output("[events]\t%.3f\n", Debug::EndSample());
	
	update_entities->Restart();
	UpdateEntities();
	update_entities->Stop();
	Debug::Output("[WorldInternal::Update::update_entities]\t%.3f\n", update_entities->GetElapsedSeconds());
	
	update_decals->Restart();
	UpdateDecals();
	update_decals->Stop();
	Debug::Output("[WorldInternal::Update::update_decals]\t%.3f\n", update_decals->GetElapsedSeconds());
	
	update_rendering->Restart();
	RenderUpdate();
	update_rendering->Stop();
	Debug::Output("[WorldInternal::Update::update_rendering]\t%.3f\n", update_rendering->GetElapsedSeconds());

	//Debug::Output("[#total]\t%.3f\n", Debug::EndSample());
}
