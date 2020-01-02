#include "sceneinternal.h"

#include <mutex>
#include "engine.h"
#include "decalcreater.h"
#include "internal/codec/gameobjectimporter.h"
#include "internal/components/transforminternal.h"
#include "internal/gameobject/gameobjectinternal.h"

Scene::Scene() : Subsystem(new SceneInternal()) {}
void Scene::GetDecals(std::vector<Decal>& container) { _suede_dptr()->GetDecals(container); }
Environment* Scene::GetEnvironment() { return _suede_dptr()->GetEnvironment(); }
GameObject* Scene::GetGameObject(uint id) { return _suede_dptr()->GetGameObject(id); }
void Scene::DestroyGameObject(uint id) { _suede_dptr()->DestroyGameObject(id); }
void Scene::DestroyGameObject(GameObject* go) { _suede_dptr()->DestroyGameObject(go); }
std::vector<GameObject*> Scene::GetGameObjectsOfComponent(suede_guid guid) { return _suede_dptr()->GetGameObjectsOfComponent(guid); }
void Scene::WalkGameObjectHierarchy(std::function<WalkCommand(GameObject*)> walker) { _suede_dptr()->WalkGameObjectHierarchy(walker); }
Transform* Scene::GetRootTransform() { return _suede_dptr()->GetRootTransform(); }
GameObject* Scene::Import(const std::string& path, std::function<void(GameObject*, const std::string&)> callback) { return _suede_dptr()->Import(path, callback); }
void Scene::Awake() { _suede_dptr()->Awake(); }
void Scene::Update(float deltaTime) { _suede_dptr()->Update(deltaTime); }
void Scene::CullingUpdate(float deltaTime) { _suede_dptr()->CullingUpdate(deltaTime); }

bool SceneInternal::LightComparer::operator()(const ref_ptr<Light>& lhs, const ref_ptr<Light>& rhs) const {
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

bool SceneInternal::CameraComparer::operator() (const ref_ptr<Camera>& lhs, const ref_ptr<Camera>& rhs) const {
	return lhs->GetDepth() < rhs->GetDepth();
}

bool SceneInternal::ProjectorComparer::operator() (const ref_ptr<Projector>& lhs, const ref_ptr<Projector>& rhs) const {
	return lhs->GetDepth() < rhs->GetDepth();
}

void SceneInternal::Awake() {
	GameObject::created.subscribe(this, &SceneInternal::AddGameObject);
	GameObjectInternal::componentChanged.subscribe(this, &SceneInternal::OnGameObjectComponentChanged);

	root_ = new GameObject("Root");

	importer_ = new GameObjectImporter();
	decalCreater_ = new DecalCreater();
}

void SceneInternal::OnDestroy() {
	GameObject::created.unsubscribe(this);
	GameObjectInternal::componentChanged.unsubscribe(this);

	for (ref_ptr<Camera>& camera : cameras_) {
		camera->OnBeforeWorldDestroyed();
	}

	Camera::SetMain(nullptr);

	delete importer_;
	delete decalCreater_;
}

void SceneInternal::Update(float deltaTime) {
	RenderingUpdateGameObjects(deltaTime);

	Camera::OnPreRender();

	for (ref_ptr<Camera>& camera : cameras_) {
		if (camera->GetEnabled()) {
			camera->Render();
		}
	}

	Camera::OnPostRender();
}

void SceneInternal::CullingUpdate(float deltaTime) {
	CullingUpdateGameObjects(deltaTime);
	UpdateDecals();
}

void SceneInternal::AddGameObject(ref_ptr<GameObject> go) {
	std::lock_guard<std::mutex> lock(TransformInternal::hierarchyMutex);
	gameObjects_.insert(std::make_pair(go->GetInstanceID(), go));
}

void SceneInternal::OnGameObjectParentChanged(GameObject* go) {
	if (go->GetTransform()->GetParent()) {
		gameObjects_.insert(std::make_pair(go->GetInstanceID(), go));
	}
	else {
		gameObjects_.erase(go->GetInstanceID());
	}
}

void SceneInternal::OnGameObjectComponentChanged(ref_ptr<GameObject> go, ComponentEventType state, ref_ptr<Component> component) {
	ManageGameObjectUpdateSequence(go.get());

	ManageGameObjectComponents(lights_, component.get(), state);
	ManageGameObjectComponents(cameras_, component.get(), state);
	ManageGameObjectComponents(projectors_, component.get(), state);
	ManageGameObjectComponents(gizmosPainters_, component.get(), state);
}

void SceneInternal::GetDecals(std::vector<Decal>& container) {
	decalCreater_->GetDecals(container);
}

void SceneInternal::UpdateDecals() {
	if (Camera::GetMain()) {
		decalCreater_->Update(Camera::GetMain(), projectors_);
	}
}

void SceneInternal::CullingUpdateGameObjects(float deltaTime) {
	for (GameObject* go : cullingUpdateSequence_) {
		if (go->GetActive()) {
			go->CullingUpdate(deltaTime);
		}
	}
}

void SceneInternal::RenderingUpdateGameObjects(float deltaTime) {
	for (GameObject* go : renderingUpdateSequence_) {
		if (go->GetActive()) {
			go->Update(deltaTime);
		}
	}
}

GameObject* SceneInternal::GetGameObject(uint id) {
	GameObjectDictionary::iterator ite = gameObjects_.find(id);
	if (ite == gameObjects_.end()) { return nullptr; }
	return ite->second.get();
}

void SceneInternal::DestroyGameObject(uint id) {
	GameObject* go = GetGameObject(id);
	if (go) {
		DestroyGameObject(go);
	}
}

void SceneInternal::DestroyGameObject(GameObject* go) {
	DestroyGameObjectRecursively(go->GetTransform());
}

void SceneInternal::DestroyGameObjectRecursively(Transform* root) {
	GameObject* go = root->GetGameObject();
	RemoveGameObject(go);

	GameObject::destroyed.delay_raise(go);

	for (int i = 0; i < root->GetChildCount(); ++i) {
		DestroyGameObjectRecursively(root->GetChildAt(i));
	}
}

void SceneInternal::RemoveGameObject(GameObject* go) {
	Camera* camera = go->GetComponent<Camera>();
	if (camera) { cameras_.erase(camera); }

	Light* light = go->GetComponent<Light>();
	if (light) { lights_.erase(light); }

	Projector* projector = go->GetComponent<Projector>();
	if (projector) { projectors_.erase(projector); }

	RemoveGameObjectFromSequence(go);
	gameObjects_.erase(go->GetInstanceID());
	go->GetTransform()->SetParent(nullptr);
}

std::vector<GameObject*> SceneInternal::GetGameObjectsOfComponent(suede_guid guid) {
	std::vector<GameObject*> gameObjects;
	if (guid == Camera::GetComponentGUID()) {
		for (ref_ptr<Camera>& camera : cameras_) {
			gameObjects.push_back(camera->GetGameObject());
		}
	}
	else if (guid == Projector::GetComponentGUID()) {
		for (const ref_ptr<Projector>& projector : projectors_) {
			gameObjects.push_back(projector->GetGameObject());
		}
	}
	else if (guid == Light::GetComponentGUID()) {
		for (const ref_ptr<Light>& light : lights_) {
			gameObjects.push_back(light->GetGameObject());
		}
	}
	else if (guid == GizmosPainter::GetComponentGUID()) {
		for (const ref_ptr<GizmosPainter>& painter : gizmosPainters_) {
			gameObjects.push_back(painter->GetGameObject());
		}
	}
	else {
		for (GameObjectDictionary::iterator ite = gameObjects_.begin(); ite != gameObjects_.end(); ++ite) {
			if (ite->second->GetComponent(guid)) {
				gameObjects.push_back(ite->second.get());
			}
		}
	}

	return gameObjects;
}

void SceneInternal::WalkGameObjectHierarchy(std::function<WalkCommand(GameObject*)> walker) {
	std::lock_guard<std::mutex> lock(TransformInternal::hierarchyMutex);
	WalkGameObjectHierarchyRecursively(GetRootTransform(), walker);
}

GameObject* SceneInternal::Import(const std::string& path, std::function<void(GameObject*, const std::string&)> callback) {
	return importer_->Import(path, callback).get();
}

bool SceneInternal::WalkGameObjectHierarchyRecursively(Transform* root, std::function<WalkCommand(GameObject*)> walker) {
	for (int i = 0; i < root->GetChildCount(); ++i) {
		Transform* transform = root->GetChildAt(i);
		GameObject* child = transform->GetGameObject();
		if (child == nullptr) {
			continue;
		}

		WalkCommand command = walker(child);

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

void SceneInternal::RemoveGameObjectFromSequence(GameObject* go) {
	cullingUpdateSequence_.erase(go);
	renderingUpdateSequence_.erase(go);
}

void SceneInternal::ManageGameObjectUpdateSequence(GameObject* go) {
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