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
void Scene::Import(const std::string& path, std::function<void(GameObject*, const std::string&)> callback) { _suede_dptr()->Import(path, callback); }
void Scene::Awake() { _suede_dptr()->Awake(); }
void Scene::Update(float deltaTime) { _suede_dptr()->Update(deltaTime); }
void Scene::CullingUpdate(float deltaTime) { _suede_dptr()->CullingUpdate(deltaTime); }

template <class Container>
inline void EraseByValue(Container& cont, const typename Container::value_type& value) {
	cont.erase(std::remove(cont.begin(), cont.end(), value), cont.end());
}

template <class Container>
inline void InsertUnique(Container& cont, const typename Container::value_type& value) {
	if (std::find(cont.begin(), cont.end(), value) == cont.end()) { cont.push_back(value); }
}

void SceneInternal::Awake() {
	GameObjectInternal::created.subscribe(this, &SceneInternal::AddGameObject);
	GameObjectInternal::componentChanged.subscribe(this, &SceneInternal::OnGameObjectComponentChanged);

	root_ = new GameObject("Root");

	importer_ = new GameObjectImporter();
	decalCreater_ = new DecalCreater();
}

void SceneInternal::OnDestroy() {
	GameObjectInternal::created.unsubscribe(this);
	GameObjectInternal::componentChanged.unsubscribe(this);

	Camera::SetMain(nullptr);

	delete importer_;
	delete decalCreater_;
}

void SceneInternal::Update(float deltaTime) {
	destroyed_.clear();

	SortComponents();

	RenderingUpdateGameObjects(deltaTime);

	Camera::OnPreRender();

	for (Camera* camera : cameras_) {
		if (camera->GetActiveAndEnabled()) {
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

void SceneInternal::OnGameObjectComponentChanged(ref_ptr<GameObject> go, ComponentEventType state, ref_ptr<Component> component) {
	ManageGameObjectUpdateSequence(go.get());

	ManageGameObjectComponents(lights_, component.get(), state);
	ManageGameObjectComponents(cameras_, component.get(), state);
	ManageGameObjectComponents(projectors_, component.get(), state);
	ManageGameObjectComponents(renderers_, component.get(), state);
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

void SceneInternal::SortComponents() {
	struct CameraComparer {
		bool operator()(Camera* lhs, Camera* rhs) const {
			return lhs->GetDepth() < rhs->GetDepth();
		}
	} static cameraComparer;
	std::stable_sort(cameras_.begin(), cameras_.end(), cameraComparer);

	struct ProjectorComparer {
		bool operator()(Projector* lhs, Projector* rhs) const {
			return lhs->GetDepth() < rhs->GetDepth();
		}
	} static projectorComparer;
	std::stable_sort(projectors_.begin(), projectors_.end(), projectorComparer);

	struct LightComparerer {
		bool operator()(Light* lhs, Light* rhs) const {
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
	} static lightComparer;
	std::stable_sort(lights_.begin(), lights_.end(), lightComparer);
}

void SceneInternal::CullingUpdateGameObjects(float deltaTime) {
	std::lock_guard<std::mutex> lock(cullingMutex_);
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
	auto ite = gameObjects_.find(id);
	return (ite != gameObjects_.end()) ? ite->second.get() : nullptr;
}

void SceneInternal::DestroyGameObject(uint id) {
	GameObject* go = GetGameObject(id);
	if (go != nullptr) {
		DestroyGameObject(go);
	}
}

void SceneInternal::DestroyGameObject(GameObject* go) {
	DestroyGameObjectRecursively(go->GetTransform());
}

void SceneInternal::DestroyGameObjectRecursively(Transform* root) {
	for (int i = 0; i < root->GetChildCount(); ++i) {
		DestroyGameObjectRecursively(root->GetChildAt(i));
	}

	RemoveGameObject(root->GetGameObject());
}

void SceneInternal::RemoveGameObject(GameObject* go) {
	Camera* camera = go->GetComponent<Camera>();
	if (camera) { EraseByValue(cameras_, camera); }

	Light* light = go->GetComponent<Light>();
	if (light != nullptr) { EraseByValue(lights_, light); }

	Projector* projector = go->GetComponent<Projector>();
	if (projector) { EraseByValue(projectors_, projector); }

	Renderer* renderer = go->GetComponent<Renderer>();
	if (renderer) { EraseByValue(renderers_, renderer); }

	RemoveGameObjectFromSequence(go);
	go->GetTransform()->SetParent(nullptr);

	destroyed_.insert(go);
	gameObjects_.erase(go->GetInstanceID());
}

std::vector<GameObject*> SceneInternal::GetGameObjectsOfComponent(suede_guid guid) {
	std::vector<GameObject*> gameObjects;
	if (guid == Renderer::GetComponentGUID()) {
		for (const ref_ptr<Renderer>& renderer : renderers_) {
			gameObjects.push_back(renderer->GetGameObject());
		}
	}
	else if (guid == Camera::GetComponentGUID()) {
		for (Camera* camera : cameras_) {
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
		for (auto ite = gameObjects_.begin(); ite != gameObjects_.end(); ++ite) {
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

void SceneInternal::Import(const std::string& path, std::function<void(GameObject*, const std::string&)> callback) {
	importer_->Import(path, callback);
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
	{
		std::lock_guard<std::mutex> lock(cullingMutex_);
		EraseByValue(cullingUpdateSequence_, go);
	}

	EraseByValue(renderingUpdateSequence_, go);
}

void SceneInternal::ManageGameObjectUpdateSequence(GameObject* go) {
	int strategy = go->GetUpdateStrategy();
	if ((strategy & UpdateStrategyCulling) != 0) {
		std::lock_guard<std::mutex> lock(cullingMutex_);
		InsertUnique(cullingUpdateSequence_, go);
	}
	else {
		std::lock_guard<std::mutex> lock(cullingMutex_);
		EraseByValue(cullingUpdateSequence_, go);
	}

	if ((strategy & UpdateStrategyRendering) != 0) {
		InsertUnique(renderingUpdateSequence_, go);
	}
	else {
		EraseByValue(renderingUpdateSequence_, go);
	}
}

template <class Container>
void SceneInternal::ManageGameObjectComponents(Container& container, Component* component, ComponentEventType state) {
	typedef typename Container::value_type V;
	typedef typename std::remove_pointer<V>::type T;

	if (component->IsComponentType(T::GetComponentGUID())) {
		T* target = (T*)component;
		if (state == ComponentEventType::Added) {
			container.insert(container.end(), target);
		}
		else if (state == ComponentEventType::Removed) {
			EraseByValue(container, target);
		}
	}
}
