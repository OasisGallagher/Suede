#include "time2.h"
#include "worldinternal.h"
#include "../api/glutils.h"
#include "geometryutility.h"
#include "internal/async/guard.h"
#include "internal/rendering/shadows.h"
#include "internal/codec/entityloader.h"
#include "internal/rendering/matrixbuffer.h"
#include "internal/entities/entityinternal.h"
#include "internal/world/environmentinternal.h"
#include "internal/components/transforminternal.h"
#include "internal/rendering/uniformbuffermanager.h"

#include "internal/tools/gizmosinternal.h"
#include "internal/tools/graphicsinternal.h"
#include "internal/tools/resourcesinternal.h"

bool WorldInternal::LightComparer::operator()(const Light& lhs, const Light& rhs) const {
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
	: importer_(MEMORY_NEW(EntityLoaderThreadPool)), decals_(SUEDE_MAX_DECALS) {
	Screen::instance()->AddScreenSizeChangedListener(this);
	AddEventListener(this);
}

void WorldInternal::Initialize() {
	GLUtils::Initialize();

	Resources::implement(new ResourcesInternal);
	Gizmos::implement(new GizmosInternal);
	Graphics::implement(new GraphicsInternal);
	Environment::implement(new EnvironmentInternal);

	UniformBufferManager::instance();

	Shadows::instance();
	MatrixBuffer::instance();

	root_ = Factory::Create<EntityInternal>();
	root_->SetTransform(Factory::Create<TransformInternal>());
	root_->SetName("Root");
}

WorldInternal::~WorldInternal() {
}

void WorldInternal::Finalize() {
	for (CameraContainer::iterator ite = cameras_.begin(); ite != cameras_.end(); ++ite) {
		(*ite)->OnBeforeWorldDestroyed();
	}

	MEMORY_DELETE(importer_);

	RemoveEventListener(this);
	Screen::instance()->RemoveScreenSizeChangedListener(this);
}

Object WorldInternal::CreateObject(ObjectType type) {
	Object object = Factory::Create(type);
	if (type >= ObjectTypeEntity) {
		Entity entity = suede_dynamic_cast<Entity>(object);
		Transform transform = Factory::Create<TransformInternal>();
		entity->SetTransform(transform);

		EntityCreatedEventPointer e = NewWorldEvent<EntityCreatedEventPointer>();
		e->entity = entity;
		FireEvent(e);

 		GUARD_SCOPE_TYPED(Transform);
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

Entity WorldInternal::Import(const std::string& path, EntityLoadedListener* listener) {
	importer_->SetLoadedListener(listener);
	return importer_->Import(path);
}

bool WorldInternal::ImportTo(Entity entity, const std::string& path, EntityLoadedListener* listener) {
	importer_->SetLoadedListener(listener);
	return importer_->ImportTo(entity, path);
}

Entity WorldInternal::GetEntity(uint id) {
	EntityDictionary::iterator ite = entities_.find(id);
	if (ite == entities_.end()) { return nullptr; }
	return ite->second;
}

void WorldInternal::DestroyEntity(uint id) {
	Entity entity = GetEntity(id);
	if (entity) {
		DestroyEntity(entity);
	}
}

void WorldInternal::DestroyEntity(Entity entity) {
	DestroyEntityRecursively(entity->GetTransform());
}

void WorldInternal::DestroyEntityRecursively(Transform root) {
	Entity entity = root->GetEntity();

	if (entity->GetType() == ObjectTypeCamera) {
		cameras_.erase(suede_dynamic_cast<Camera>(entity));
	}
	else if (entity->GetType() == ObjectTypeProjector) {
		projectors_.erase(suede_dynamic_cast<Projector>(entity));
	}
	else if (entity->GetType() >= ObjectTypeSpotLight && entity->GetType() <= ObjectTypeDirectionalLight) {
		lights_.erase(suede_dynamic_cast<Light>(entity));
	}

	// TODO: tag entity with destroyed?
	RemoveEntityFromSequence(entity);
	entities_.erase(entity->GetInstanceID());
	entity->GetTransform()->SetParent(nullptr);

	EntityDestroyedEventPointer e = NewWorldEvent<EntityDestroyedEventPointer>();
	e->entity = entity;
	FireEvent(e);

	for(Transform transform : root->GetChildren()) {
		DestroyEntityRecursively(transform);
	}
}

bool WorldInternal::GetEntities(ObjectType type, std::vector<Entity>& entities) {
	if (type < ObjectTypeEntity) {
		Debug::LogError("invalid entity type");
		return false;
	}

	if (type == ObjectTypeEntity) {
		for (EntityDictionary::iterator ite = entities_.begin(); ite != entities_.end(); ++ite) {
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
		for (EntityDictionary::iterator ite = entities_.begin(); ite != entities_.end(); ++ite) {
			if (ite->second->GetType() == type) {
				entities.push_back(ite->second);
			}
		}
	}

	return !entities.empty();
}

void WorldInternal::WalkEntityHierarchy(WorldEntityWalker* walker) {
	GUARD_SCOPE_TYPED(Transform);
	WalkEntityHierarchyRecursively(GetRootTransform(), walker);
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

	GUARD_SCOPE_TYPED(WorldEventContainer);
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

void WorldInternal::UpdateDecals() {
	decals_.clear();

	Camera main = Camera::GetMain();
	if (main) { CreateDecals(main); }
}

void WorldInternal::CullingUpdateEntities() {
	for (Entity entity : cullingUpdateSequence_) {
		if (entity->GetActive()) {
			entity->CullingUpdate();
		}
	}
}

void WorldInternal::RenderingUpdateEntities() {
	for (Entity entity : renderingUpdateSequence_) {
		if (entity->GetActive()) {
			entity->RenderingUpdate();
		}
	}
}

bool WorldInternal::WalkEntityHierarchyRecursively(Transform root, WorldEntityWalker* walker) {
	for(Transform transform : root->GetChildren()) {
		Entity child = transform->GetEntity();
		if (!child) {
			continue;
		}

		WorldEntityWalker::WalkCommand command = walker->OnWalkEntity(child);

		// next sibling.
		if (command == WorldEntityWalker::WalkCommandNext) {
			continue;
		}

		// 
		if (command == WorldEntityWalker::WalkCommandBreak) {
			return false;
		}

		if (!WalkEntityHierarchyRecursively(child->GetTransform(), walker)) {
			return false;
		}
	}

	return true;
}

void WorldInternal::OnScreenSizeChanged(uint width, uint height) {
}

void WorldInternal::OnWorldEvent(WorldEventBasePointer e) {
	if (e->GetEventType() == WorldEventTypeCameraDepthChanged) {
		cameras_.sort();
	}
	else if (e->GetEventType() == WorldEventTypeEntityParentChanged) {
		Entity entity = suede_static_cast<EntityParentChangedEventPointer>(e)->entity;
		if (entity->GetTransform()->GetParent()) {
			entities_.insert(std::make_pair(entity->GetInstanceID(), entity));
		}
		else {
			entities_.erase(entity->GetInstanceID());
		}
	}
	else if (e->GetEventType() == WorldEventTypeEntityUpdateStrategyChanged) {
		Entity entity = suede_static_cast<EntityUpdateStrategyChangedEventPointer>(e)->entity;
		AddEntityToUpdateSequence(entity);
	}
}

void WorldInternal::FireEvents() {
	for (uint i = 0; i < WorldEventTypeCount; ++i) {
		WorldEventCollection collection = events_[i];
		for (WorldEventCollection::const_iterator ite = collection.begin(); ite != collection.end(); ++ite) {
			FireEventImmediate(*ite);
		}
	}

	GUARD_SCOPE_TYPED(WorldEventContainer);
	for (uint i = 0; i < WorldEventTypeCount; ++i) {
		events_[i].clear();
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
	decal.topology = MeshTopology::Triangles;

	return true;
}

bool WorldInternal::CreateProjectorDecal(Camera camera, Projector p, Plane planes[6]) {
	/*std::vector<Entity> entities;
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
	}*/
	
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

void WorldInternal::UpdateTimeUniformBuffer() {
	static SharedTimeUniformBuffer p;
	p.time.x = Time::instance()->GetRealTimeSinceStartup();
	p.time.y = Time::instance()->GetDeltaTime();
	UniformBufferManager::instance()->UpdateSharedBuffer(SharedTimeUniformBuffer::GetName(), &p, 0, sizeof(p));
}

void WorldInternal::RemoveEntityFromSequence(Entity entity) {
	cullingUpdateSequence_.erase(entity);
	renderingUpdateSequence_.erase(entity);
}

void WorldInternal::AddEntityToUpdateSequence(Entity entity) {
	int strategy = entity->GetUpdateStrategy();
	if ((strategy & UpdateStrategyCulling) != 0) {
		if (!cullingUpdateSequence_.contains(entity)) {
			cullingUpdateSequence_.insert(entity);
		}
	}
	else {
		cullingUpdateSequence_.erase(entity);
	}

	if ((strategy & UpdateStrategyRendering) != 0) {
		if (!renderingUpdateSequence_.contains(entity)) {
			renderingUpdateSequence_.insert(entity);
		}
	}
	else {
		renderingUpdateSequence_.erase(entity);
	}
}

void WorldInternal::CullingUpdate() {
	UpdateDecals();
	CullingUpdateEntities();
}

void WorldInternal::RenderingUpdate() {
	FireEvents();

	RenderingUpdateEntities();

	// TODO: CLEAR STENCIL BUFFER.
	//Framebuffer0::Get()->Clear(FramebufferClearMaskColorDepthStencil);

	UpdateTimeUniformBuffer();

	for (CameraContainer::iterator ite = cameras_.begin(); ite != cameras_.end(); ++ite) {
		if ((*ite)->GetActive()) {
			(*ite)->Render();
		}
	}
}
