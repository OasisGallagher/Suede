#pragma once
#include <map>

#include "scene.h"
#include "engine.h"
#include "light.h"
#include "camera.h"
#include "renderer.h"
#include "projector.h"
#include "gameobject.h"
#include "gizmospainter.h"
#include "subsysteminternal.h"
#include "containers/sortedvector.h"

class DecalCreater;
class GameObjectImporter;

class SceneInternal : public SubsystemInternal {
public:
	void GetDecals(std::vector<Decal>& container);
	Environment* GetEnvironment() { return &environment_; }

	GameObject* GetGameObject(uint id);
	void DestroyGameObject(uint id);
	void DestroyGameObject(GameObject* go);
	std::vector<GameObject*> GetGameObjectsOfComponent(suede_guid guid);
	void WalkGameObjectHierarchy(std::function<WalkCommand(GameObject*)> walker);
	Transform* GetRootTransform() { return root_->GetTransform(); }
	void Import(const std::string& path, std::function<void(GameObject*, const std::string&)> callback);

public:
	void Awake();
	void Update(float deltaTime);

	void CullingUpdate(float deltaTime);
	void OnDestroy();

private:
	void UpdateDecals();
	void SortComponents();
	void CullingUpdateGameObjects(float deltaTime);
	void RenderingUpdateGameObjects(float deltaTime);
	void AddGameObject(ref_ptr<GameObject> go);
	void RemoveGameObject(GameObject* go);
	void RemoveGameObjectFromSequence(GameObject* go);
	void ManageGameObjectUpdateSequence(GameObject* go);
	void DestroyGameObjectRecursively(Transform* root);
	bool WalkGameObjectHierarchyRecursively(Transform* root, std::function<WalkCommand(GameObject*)> walker);

	void OnGameObjectComponentChanged(ref_ptr<GameObject> go, ComponentEventType state, ref_ptr<Component> component);

	template <class Container>
	void ManageGameObjectComponents(Container& container, Component* component, ComponentEventType state);

private:
	std::vector<Light*> lights_;
	std::vector<Camera*> cameras_;
	std::vector<Renderer*> renderers_;
	std::vector<Projector*> projectors_;
	std::map<uint, ref_ptr<GameObject>> gameObjects_;
	std::vector<GizmosPainter*> gizmosPainters_;

	std::mutex cullingMutex_;
	std::vector<GameObject*> cullingUpdateSequence_;
	std::vector<GameObject*> renderingUpdateSequence_;

	Environment environment_;
	DecalCreater* decalCreater_;
	std::set<ref_ptr<GameObject>> destroyed_;

	ref_ptr<GameObject> root_;
	GameObjectImporter* importer_;
};
