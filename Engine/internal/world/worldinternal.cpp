#include "worldinternal.h"

#include "time2.h"
#include "profiler.h"
#include "statistics.h"
#include "decalcreater.h"
#include "../api/glutils.h"

#include "internal/tools/gizmosinternal.h"
#include "internal/tools/graphicsinternal.h"
#include "internal/tools/resourcesinternal.h"

#include "internal/async/guard.h"
#include "internal/rendering/shadows.h"
#include "internal/codec/gameObjectloader.h"
#include "internal/rendering/matrixbuffer.h"
#include "internal/world/environmentinternal.h"
#include "internal/components/transforminternal.h"
#include "internal/gameobject/gameobjectinternal.h"
#include "internal/rendering/uniformbuffermanager.h"

World::World() : Singleton2<World>(MEMORY_NEW(WorldInternal), Memory::DeleteRaw<WorldInternal>) {}
World::~World() {}

void World::Initialize() { _suede_dptr()->Initialize(); }
void World::CullingUpdate() { _suede_dptr()->CullingUpdate(); }
void World::RenderingUpdate() { _suede_dptr()->RenderingUpdate(); }
void World::Finalize() { _suede_dptr()->Finalize(); }
Object World::CreateObject(ObjectType type) { return _suede_dptr()->CreateObject(type); }
void World::DestroyGameObject(uint id) { _suede_dptr()->DestroyGameObject(id); }
void World::DestroyGameObject(GameObject go) { _suede_dptr()->DestroyGameObject(go); }
GameObject World::Import(const std::string& path, GameObjectImportedListener* listener) { return _suede_dptr()->Import(path, listener); }
GameObject World::Import(const std::string& path, Lua::Func<void, GameObject, const std::string&> callback) { return _suede_dptr()->Import(path, callback); }
bool World::ImportTo(GameObject go, const std::string& path, GameObjectImportedListener* listener) { return _suede_dptr()->ImportTo(go, path,listener); }
Transform World::GetRootTransform() { return _suede_dptr()->GetRootTransform(); }
GameObject World::GetGameObject(uint id) { return _suede_dptr()->GetGameObject(id); }
void World::WalkGameObjectHierarchy(WorldGameObjectWalker* walker) { _suede_dptr()->WalkGameObjectHierarchy(walker); }
void World::FireEvent(WorldEventBasePtr e) { _suede_dptr()->FireEvent(e); }
void World::FireEventImmediate(WorldEventBasePtr e) { _suede_dptr()->FireEventImmediate(e); }
void World::AddEventListener(WorldEventListener* listener) { _suede_dptr()->AddEventListener(listener); }
void World::RemoveEventListener(WorldEventListener* listener) { _suede_dptr()->RemoveEventListener(listener); }
void World::GetDecals(std::vector<Decal>& container) { _suede_dptr()->GetDecals(container); }
std::vector<GameObject> World::GetGameObjectsOfComponent(suede_guid guid) { return _suede_dptr()->GetGameObjectsOfComponent(guid); }

bool WorldInternal::LightComparer::operator()(const Light& lhs, const Light& rhs) const {
	// Directional light > Importance > Luminance.
	LightType lt = lhs->GetType(), rt = rhs->GetType();
	if (lt != rt && (lt == LightType::Directional || rt == LightType::Directional)) {
		return lt == LightType::Directional;
	}

	LightImportance lli = lhs->GetImportance(), rli = rhs->GetImportance();
	if (lli != rli) {
		return lli > rli;
	}

	return lhs->GetColor().GetLuminance() > rhs->GetColor().GetLuminance();
}

bool WorldInternal::CameraComparer::operator() (const Camera& lhs, const Camera& rhs) const {
	return lhs->GetDepth() < rhs->GetDepth();
}

bool WorldInternal::ProjectorComparer::operator() (const Projector& lhs, const Projector& rhs) const {
	return lhs->GetDepth() < rhs->GetDepth();
}

WorldInternal::WorldInternal()
	: importer_(MEMORY_NEW(GameObjectLoaderThreadPool)) {
	Screen::instance()->AddScreenSizeChangedListener(this);
	AddEventListener(this);
}

void WorldInternal::Initialize() {
	GLUtils::Initialize();
	Resources::instance()->FindShader("builtin/lit_texture");

	UniformBufferManager::instance();
	Shadows::instance();
	MatrixBuffer::instance();

	decalCreater_ = MEMORY_NEW(DecalCreater);

	root_ = Factory::Create<GameObject>();
	root_->AddComponent<ITransform>();
	root_->SetName("Root");
}

WorldInternal::~WorldInternal() {
}

void WorldInternal::Finalize() {
	for (Camera camera : cameras_) {
		camera->OnBeforeWorldDestroyed();
	}

	Camera::main(nullptr);

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

GameObject WorldInternal::Import(const std::string& path, GameObjectImportedListener* listener) {
	importer_->SetImportedListener(listener);
	return importer_->Import(path);
}

GameObject WorldInternal::Import(const std::string& path, Lua::Func<void, GameObject, const std::string&> callback) {
	importer_->SetImportedCallback(callback);
	return importer_->Import(path);
}

bool WorldInternal::ImportTo(GameObject go, const std::string& path, GameObjectImportedListener* listener) {
	importer_->SetImportedListener(listener);
	return importer_->ImportTo(go, path);
}

GameObject WorldInternal::GetGameObject(uint id) {
	GameObjectDictionary::iterator ite = gameObjects_.find(id);
	if (ite == gameObjects_.end()) { return nullptr; }
	return ite->second;
}

void WorldInternal::DestroyGameObject(uint id) {
	GameObject go = GetGameObject(id);
	if (go) {
		DestroyGameObject(go);
	}
}

void WorldInternal::DestroyGameObject(GameObject go) {
	DestroyGameObjectRecursively(go->GetTransform());
}

void WorldInternal::DestroyGameObjectRecursively(Transform root) {
	GameObject go = root->GetGameObject();

	Camera camera = go->GetComponent<ICamera>();
	if (camera) { cameras_.erase(camera); }

	Light light = go->GetComponent<ILight>();
	if (light) { lights_.erase(light); }

	Projector projector = go->GetComponent<IProjector>();
	if (projector) { projectors_.erase(projector); }

	RemoveGameObjectFromSequence(go);
	gameObjects_.erase(go->GetInstanceID());
	go->GetTransform()->SetParent(nullptr);

	GameObjectDestroyedEventPtr e = NewWorldEvent<GameObjectDestroyedEventPtr>();
	e->go = go;
	FireEvent(e);

	for(Transform transform : root->GetChildren()) {
		DestroyGameObjectRecursively(transform);
	}
}

std::vector<GameObject> WorldInternal::GetGameObjectsOfComponent(suede_guid guid) {
	std::vector<GameObject> gameObjects;
	if (guid == ICamera::GetComponentGUID()) {
		for (Camera camera : cameras_) {
			gameObjects.push_back(camera->GetGameObject());
		}
	}
	else if (guid == IProjector::GetComponentGUID()) {
		for (Projector projector : projectors_) {
			gameObjects.push_back(projector->GetGameObject());
		}
	}
	else if (guid == ILight::GetComponentGUID()) {
		for (Light light : lights_) {
			gameObjects.push_back(light->GetGameObject());
		}
	}
	else if (guid == GizmosPainter::GetComponentGUID()) {
		for (GizmosPainterPtr painter : gizmosPainters_) {
			gameObjects.push_back(painter->GetGameObject());
		}
	}
	else {
		for (GameObjectDictionary::iterator ite = gameObjects_.begin(); ite != gameObjects_.end(); ++ite) {
			if (ite->second->GetComponent(guid)) {
				gameObjects.push_back(ite->second);
			}
		}
	}

	return gameObjects;
}

void WorldInternal::WalkGameObjectHierarchy(WorldGameObjectWalker* walker) {
	ZTHREAD_LOCK_SCOPE(TransformInternal::hierarchyMutex);
	WalkGameObjectHierarchyRecursively(GetRootTransform(), walker);
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

void WorldInternal::FireEvent(WorldEventBasePtr e) {
	WorldEventType type = e->GetEventType();
	WorldEventCollection& collection = events_[(int)type];

	ZTHREAD_LOCK_SCOPE(eventsMutex_);
	collection.push_back(e);
}

void WorldInternal::FireEventImmediate(WorldEventBasePtr e) {
	for (WorldEventListener* listener : listeners_) {
		listener->OnWorldEvent(e);
	}
}

void WorldInternal::GetDecals(std::vector<Decal>& container) {
	decalCreater_->GetDecals(container);
}

void WorldInternal::UpdateDecals() {
	if (Camera::main()) {
		decalCreater_->Update(Camera::main(), projectors_);
	}
}

void WorldInternal::CullingUpdateGameObjects() {
	for (GameObject go : cullingUpdateSequence_) {
		if (go->GetActive()) {
			go->CullingUpdate();
		}
	}
}

void WorldInternal::RenderingUpdateGameObjects() {
	for (GameObject go : renderingUpdateSequence_) {
		if (go->GetActive()) {
			go->RenderingUpdate();
		}
	}
}

bool WorldInternal::WalkGameObjectHierarchyRecursively(Transform root, WorldGameObjectWalker* walker) {
	for(Transform transform : root->GetChildren()) {
		GameObject child = transform->GetGameObject();
		if (!child) {
			continue;
		}

		WalkCommand command = walker->OnWalkGameObject(child);

		// next sibling.
		if (command == WalkCommand::Next) {
			continue;
		}

		// 
		if (command == WalkCommand::Break) {
			return false;
		}

		if (!WalkGameObjectHierarchyRecursively(child->GetTransform(), walker)) {
			return false;
		}
	}

	return true;
}

void WorldInternal::OnScreenSizeChanged(uint width, uint height) {
}

void WorldInternal::OnWorldEvent(WorldEventBasePtr e) {
	switch (e->GetEventType()) {
		case WorldEventType::CameraDepthChanged:
			cameras_.sort();
			break;
		case WorldEventType::GameObjectParentChanged:
			OnGameObjectParentChanged(suede_static_cast<GameObjectEventPtr>(e)->go);
			break;
		case WorldEventType::GameObjectUpdateStrategyChanged:
			ManageGameObjectUpdateSequence(suede_static_cast<GameObjectEventPtr>(e)->go);
			break;
		case WorldEventType::GameObjectComponentChanged:
			OnGameObjectComponentChanged(suede_static_cast<GameObjectComponentChangedEventPtr>(e));
			break;
	}
}

void WorldInternal::AddObject(Object object) {
	ObjectType type = object->GetObjectType();
	if (type == ObjectType::GameObject) {
		// add default component and fire event.
		GameObject go = suede_dynamic_cast<GameObject>(object);
		go->AddComponent<ITransform>();

		GameObjectCreatedEventPtr e = NewWorldEvent<GameObjectCreatedEventPtr>();
		e->go = go;
		FireEvent(e);

		ZTHREAD_LOCK_SCOPE(TransformInternal::hierarchyMutex);
		gameObjects_.insert(std::make_pair(go->GetInstanceID(), go));
	}
}

void WorldInternal::OnGameObjectParentChanged(GameObject go) {
	if (go->GetTransform()->GetParent()) {
		gameObjects_.insert(std::make_pair(go->GetInstanceID(), go));
	}
	else {
		gameObjects_.erase(go->GetInstanceID());
	}
}

void WorldInternal::OnGameObjectComponentChanged(GameObjectComponentChangedEventPtr e) {
	ManageGameObjectUpdateSequence(e->go);
	ManageGameObjectComponents(lights_, e->component, e->added);
	ManageGameObjectComponents(cameras_, e->component, e->added);
	ManageGameObjectComponents(projectors_, e->component, e->added);
	ManageGameObjectComponents(gizmosPainters_, e->component, e->added);
}

void WorldInternal::FireEvents() {
	ZTHREAD_LOCK_SCOPE(eventsMutex_);

	for (WorldEventCollection& collection : events_) {
		for (WorldEventBasePtr pointer : collection) {
			FireEventImmediate(pointer);
		}
	}

	for (WorldEventCollection& collection : events_) {
		collection.clear();
	}
}

void WorldInternal::UpdateTimeUniformBuffer() {
	static SharedTimeUniformBuffer p;
	p.time.x = Time::instance()->GetRealTimeSinceStartup();
	p.time.y = Time::instance()->GetDeltaTime();
	UniformBufferManager::instance()->Update(SharedTimeUniformBuffer::GetName(), &p, 0, sizeof(p));
}

void WorldInternal::RemoveGameObjectFromSequence(GameObject go) {
	cullingUpdateSequence_.erase(go);
	renderingUpdateSequence_.erase(go);
}

void WorldInternal::ManageGameObjectUpdateSequence(GameObject go) {
	int strategy = go->GetUpdateStrategy();
	if ((strategy & UpdateStrategyCulling) != 0) {
		if (!cullingUpdateSequence_.contains(go)) {
			cullingUpdateSequence_.insert(go);
		}
	}
	else {
		cullingUpdateSequence_.erase(go);
	}

	if ((strategy & UpdateStrategyRendering) != 0) {
		if (!renderingUpdateSequence_.contains(go)) {
			renderingUpdateSequence_.insert(go);
		}
	}
	else {
		renderingUpdateSequence_.erase(go);
	}
}

void WorldInternal::CullingUpdate() {
	CullingUpdateGameObjects();
}

void WorldInternal::RenderingUpdate() {
	uint64 start = Profiler::instance()->GetTimeStamp();
	FireEvents();

	// SUEDE TODO: update decals in rendering thread ?
	UpdateDecals();

	RenderingUpdateGameObjects();

	UpdateTimeUniformBuffer();

	Camera::OnPreRender();

	for (Camera camera : cameras_) {
		if (camera->GetEnabled()) {
			camera->Render();
		}
	}

	Camera::OnPostRender();

	Statistics::instance()->SetRenderingElapsed(
		Profiler::instance()->TimeStampToSeconds(Profiler::instance()->GetTimeStamp() - start)
	);
}
