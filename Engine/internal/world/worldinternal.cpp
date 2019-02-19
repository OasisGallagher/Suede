#include "worldinternal.h"

#include "time2.h"
#include "profiler.h"
#include "statistics.h"
#include "decalcreater.h"
#include "../api/glutils.h"

#include "resources.h"

#include "internal/async/async.h"
#include "internal/codec/gameObjectloader.h"
#include "internal/components/transforminternal.h"
#include "internal/gameobject/gameobjectinternal.h"

World::World() : singleton2<World>(MEMORY_NEW(WorldInternal), Memory::DeleteRaw<WorldInternal>) {}
void World::Initialize() { _suede_dinstance()->Initialize(); }
void World::Finalize() { _suede_dinstance()->Finalize(); }
void World::CullingUpdate() { _suede_dinstance()->CullingUpdate(); }
void World::Update() { _suede_dinstance()->Update(); }
void World::DestroyObject(Object object) { _suede_dinstance()->DestroyObject(object); }
GameObject World::Import(const std::string& path, GameObjectImportedListener* listener) { return _suede_dinstance()->Import(path, listener); }
GameObject World::Import(const std::string& path, Lua::Func<void, GameObject, const std::string&> callback) { return _suede_dinstance()->Import(path, callback); }
bool World::ImportTo(GameObject go, const std::string& path, GameObjectImportedListener* listener) { return _suede_dinstance()->ImportTo(go, path,listener); }
Transform World::GetRootTransform() { return _suede_dinstance()->GetRootTransform(); }
GameObject World::GetGameObject(uint id) { return _suede_dinstance()->GetGameObject(id); }
void World::WalkGameObjectHierarchy(WorldGameObjectWalker* walker) { _suede_dinstance()->WalkGameObjectHierarchy(walker); }
void World::FireEvent(WorldEventBase* e) { _suede_dinstance()->FireEvent(e); }
void World::FireEventImmediate(WorldEventBase& e) { _suede_dinstance()->FireEventImmediate(e); }
void World::AddEventListener(WorldEventListener* listener) { _suede_dinstance()->AddEventListener(listener); }
void World::RemoveEventListener(WorldEventListener* listener) { _suede_dinstance()->RemoveEventListener(listener); }
void World::GetDecals(std::vector<Decal>& container) { _suede_dinstance()->GetDecals(container); }
std::vector<GameObject> World::GetGameObjectsOfComponent(suede_guid guid) { return _suede_dinstance()->GetGameObjectsOfComponent(guid); }

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
	Screen::AddScreenSizeListener(this);
	AddEventListener(this);
}

void WorldInternal::Initialize() {
	GLUtils::Initialize();
	Resources::FindShader("builtin/lit_texture");

	decalCreater_ = MEMORY_NEW(DecalCreater);

	root_ = new IGameObject();
	root_->SetName("Root");
}

WorldInternal::~WorldInternal() {
}

void WorldInternal::Finalize() {
	for (Camera camera : cameras_) {
		camera->OnBeforeWorldDestroyed();
	}

	CameraUtility::SetMain(nullptr);

	root_.reset();
	lights_.clear();
	cameras_.clear();
	gizmosPainters_.clear();
	projectors_.clear();
	cullingUpdateSequence_.clear();
	renderingUpdateSequence_.clear();
	gameObjects_.clear();

	MEMORY_DELETE(importer_);
	MEMORY_DELETE(decalCreater_);

	RemoveEventListener(this);
	Screen::RemoveScreenSizeListener(this);
}

GameObject WorldInternal::Import(const std::string& path, GameObjectImportedListener* listener) {
	importer_->SetImportedListener(listener);
	return importer_->Import(path, nullptr);
}

GameObject WorldInternal::Import(const std::string& path, Lua::Func<void, GameObject, const std::string&> callback) {
	return importer_->Import(path, callback);
}

bool WorldInternal::ImportTo(GameObject go, const std::string& path, GameObjectImportedListener* listener) {
	importer_->SetImportedListener(listener);
	return importer_->ImportTo(go, path, nullptr);
}

GameObject WorldInternal::GetGameObject(uint id) {
	GameObjectDictionary::iterator ite = gameObjects_.find(id);
	return (ite != gameObjects_.end()) ? ite->second : nullptr;
}

void WorldInternal::DestroyObject(Object object) {
	GameObject go = suede_dynamic_cast<GameObject>(object);
	if (go) {
		return DestroyGameObject(go);
	}

	Component component = suede_dynamic_cast<Component>(object);
	if (component) {
		return component->Destroy();
	}
}

void WorldInternal::DestroyGameObject(GameObject go) {
	go->GetTransform()->SetParent(nullptr);
	DestroyGameObjectRecursively(go);
}

void WorldInternal::DestroyGameObjectRecursively(GameObject go) {
	RemoveGameObject(go);

	FireEventImmediate(GameObjectDestroyedEvent(go));

	for (Transform transform : go->GetTransform()->GetChildren()) {
		DestroyGameObjectRecursively(transform->GetGameObject());
	}

	go->Destroy();
}

void WorldInternal::RemoveGameObject(GameObject go) {
	Camera camera = go->GetComponent<Camera>();
	if (camera) { cameras_.erase(camera); }

	Light light = go->GetComponent<Light>();
	if (light) { lights_.erase(light); }

	Projector projector = go->GetComponent<Projector>();
	if (projector) { projectors_.erase(projector); }

	RemoveGameObjectFromSequence(go);
	gameObjects_.erase(go->GetInstanceID());
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
	else if (guid == IGizmosPainter::GetComponentGUID()) {
		for (GizmosPainter painter : gizmosPainters_) {
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

void WorldInternal::FireEvent(WorldEventBase* e) {
	WorldEventType type = e->GetEventType();
	WorldEventCollection& collection = events_[type];

	ZTHREAD_LOCK_SCOPE(eventsMutex_);
	collection.push_back(e);
}

void WorldInternal::FireEventImmediate(WorldEventBase& e) {
	for (WorldEventListener* listener : listeners_) {
		listener->OnWorldEvent(&e);
	}
}

void WorldInternal::GetDecals(std::vector<Decal>& container) {
	decalCreater_->GetDecals(container);
}

void WorldInternal::UpdateDecals() {
	if (!projectors_.empty() && CameraUtility::GetMain()) {
		decalCreater_->Update(CameraUtility::GetMain(), projectors_);
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
			go->Update();
		}
	}
}

void WorldInternal::PreRenderUpdateGameObjects() {
	for (GameObject go : renderingUpdateSequence_) {
		if (go->GetActive()) {
			go->OnPreRender();
		}
	}
}

void WorldInternal::PostRenderUpdateGameObjects() {
	for (GameObject go : renderingUpdateSequence_) {
		if (go->GetActive()) {
			go->OnPostRender();
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

void WorldInternal::OnWorldEvent(WorldEventBase* e) {
	switch (e->GetEventType()) {
		case WorldEventType::GameObjectCreated:
			AddGameObject(((GameObjectCreatedEvent*)e)->go);
			break;
		case WorldEventType::CameraDepthChanged:
			cameras_.sort();
			break;
		case WorldEventType::GameObjectParentChanged:
			OnGameObjectParentChanged(((GameObjectEvent*)e)->go);
			break;
		case WorldEventType::GameObjectUpdateStrategyChanged:
			ManageGameObjectUpdateSequence(((GameObjectEvent*)e)->go);
			break;
		case WorldEventType::GameObjectComponentChanged:
			OnGameObjectComponentChanged((GameObjectComponentChangedEvent*)e);
			break;
	}
}

void WorldInternal::AddGameObject(GameObject go) {
	go->AddComponent<Transform>();

	ZTHREAD_LOCK_SCOPE(TransformInternal::hierarchyMutex);
	gameObjects_.insert(std::make_pair(go->GetInstanceID(), go));
}

void WorldInternal::OnGameObjectParentChanged(GameObject go) {
	if (go->GetTransform()->GetParent()) {
		gameObjects_.insert(std::make_pair(go->GetInstanceID(), go));
	}
	else {
		gameObjects_.erase(go->GetInstanceID());
	}
}

void WorldInternal::OnGameObjectComponentChanged(GameObjectComponentChangedEvent* e) {
	ManageGameObjectUpdateSequence(e->go);

	ManageGameObjectComponents(lights_, e->component, e->state);
	ManageGameObjectComponents(cameras_, e->component, e->state);
	ManageGameObjectComponents(projectors_, e->component, e->state);
	ManageGameObjectComponents(gizmosPainters_, e->component, e->state);
}

void WorldInternal::FireEvents() {
	ZTHREAD_LOCK_SCOPE(eventsMutex_);

	for (WorldEventCollection& collection : events_) {
		for (intrusive_ptr<WorldEventBase>& pointer : collection) {
			FireEventImmediate(*pointer);
		}
	}

	for (WorldEventCollection& collection : events_) {
		collection.clear();
	}
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

void WorldInternal::Update() {
	FireEvents();

	// SUEDE TODO: update decals in rendering thread ?
	UpdateDecals();

	RenderingUpdateGameObjects();

	CameraUtility::OnPreRender();

	PreRenderUpdateGameObjects();

	for (Camera camera : cameras_) {
		if (camera->GetEnabled()) {
			camera->Render();
		}
	}

	PostRenderUpdateGameObjects();

	CameraUtility::OnPostRender();
}
