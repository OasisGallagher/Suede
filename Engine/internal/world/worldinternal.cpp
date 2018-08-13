#include "time2.h"
#include "tools/math2.h"
#include "decalcreater.h"
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
	: importer_(MEMORY_NEW(EntityLoaderThreadPool)) {
	Screen::instance()->AddScreenSizeChangedListener(this);
	AddEventListener(this);
}

void WorldInternal::Initialize() {
	GLUtils::Initialize();

	Resources::implement(new ResourcesInternal);
	
	// TODO: TEST.
	Resources::instance()->FindShader("builtin/ssao");

	Gizmos::implement(new GizmosInternal);
	Graphics::implement(new GraphicsInternal);
	Environment::implement(new EnvironmentInternal);

	UniformBufferManager::instance();
	Shadows::instance();
	MatrixBuffer::instance();

	decalCreater_ = MEMORY_NEW(DecalCreater);

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
	MEMORY_DELETE(decalCreater_);

	RemoveEventListener(this);
	Screen::instance()->RemoveScreenSizeChangedListener(this);
}

Object WorldInternal::CreateObject(ObjectType type) {
	Object object = Factory::Create(type);
	AddObject(object);
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

	return CollectEntities(type, entities);
}

void WorldInternal::WalkEntityHierarchy(WorldEntityWalker* walker) {
	ZTHREAD_LOCK_SCOPE(TransformInternal::hierarchyMutex);
	WalkEntityHierarchyRecursively(GetRootTransform(), walker);
}

void WorldInternal::AddEventListener(WorldEventListener* listener) {
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

	ZTHREAD_LOCK_SCOPE(eventsMutex_);
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

void WorldInternal::GetDecals(std::vector<Decal>& container) {
	decalCreater_->GetDecals(container);
}

void WorldInternal::UpdateDecals() {
	Camera main = Camera::GetMain();
	if (main) {
		decalCreater_->Update(main, projectors_);
	}
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
	switch (e->GetEventType()) {
		case WorldEventTypeCameraDepthChanged:
			cameras_.sort();
			break;
		case WorldEventTypeEntityParentChanged:
			OnEntityParentChanged(suede_static_cast<EntityEventPointer>(e)->entity);
			break;
		case WorldEventTypeEntityUpdateStrategyChanged:
			AddEntityToUpdateSequence(suede_static_cast<EntityEventPointer>(e)->entity);
			break;
	}
}

void WorldInternal::AddObject(Object object) {
	ObjectType type = object->GetType();
	if (type >= ObjectTypeEntity) {
		Entity entity = suede_dynamic_cast<Entity>(object);
		Transform transform = Factory::Create<TransformInternal>();
		entity->SetTransform(transform);

		EntityCreatedEventPointer e = NewWorldEvent<EntityCreatedEventPointer>();
		e->entity = entity;
		FireEvent(e);

		ZTHREAD_LOCK_SCOPE(TransformInternal::hierarchyMutex);
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
}

bool WorldInternal::CollectEntities(ObjectType type, std::vector<Entity> &entities) {
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

void WorldInternal::OnEntityParentChanged(Entity entity) {
	if (entity->GetTransform()->GetParent()) {
		entities_.insert(std::make_pair(entity->GetInstanceID(), entity));
	}
	else {
		entities_.erase(entity->GetInstanceID());
	}
}

void WorldInternal::FireEvents() {
	for (uint i = 0; i < WorldEventTypeCount; ++i) {
		WorldEventCollection collection = events_[i];
		for (WorldEventCollection::const_iterator ite = collection.begin(); ite != collection.end(); ++ite) {
			FireEventImmediate(*ite);
		}
	}

	ZTHREAD_LOCK_SCOPE(eventsMutex_);
	for (uint i = 0; i < WorldEventTypeCount; ++i) {
		events_[i].clear();
	}
}

void WorldInternal::UpdateTimeUniformBuffer() {
	static SharedTimeUniformBuffer p;
	p.time.x = Time::instance()->GetRealTimeSinceStartup();
	p.time.y = Time::instance()->GetDeltaTime();
	UniformBufferManager::instance()->Update(SharedTimeUniformBuffer::GetName(), &p, 0, sizeof(p));
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
	CullingUpdateEntities();
}

void WorldInternal::RenderingUpdate() {
	FireEvents();

	// TODO: update decals in rendering thread ?
	UpdateDecals();

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
