#pragma once
#include <map>

#include "scene.h"
#include "engine.h"
#include "light.h"
#include "camera.h"
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
	GameObject* Import(const std::string& path, std::function<void(GameObject*, const std::string&)> callback);

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
	void OnGameObjectParentChanged(GameObject* go);
	void RemoveGameObjectFromSequence(GameObject* go);
	void ManageGameObjectUpdateSequence(GameObject* go);
	void DestroyGameObjectRecursively(Transform* root);
	bool WalkGameObjectHierarchyRecursively(Transform* root, std::function<WalkCommand(GameObject*)> walker);

	void OnGameObjectComponentChanged(ref_ptr<GameObject> go, ComponentEventType state, ref_ptr<Component> component);

	template <class Container>
	void ManageGameObjectComponents(Container& container, Component* component, ComponentEventType state);

private:
	typedef std::vector<Light*> LightContainer;
	typedef std::vector<Camera*> CameraContainer;
	typedef std::vector<Projector*> ProjectorContainer;
	typedef std::vector<GameObject*> GameObjectSequence;
	typedef std::vector<GizmosPainter*> GizmosPainterContainer;

	typedef std::map<uint, ref_ptr<GameObject>> GameObjectDictionary;

private:
	LightContainer lights_;
	CameraContainer cameras_;
	DecalCreater* decalCreater_;
	ProjectorContainer projectors_;
	GameObjectDictionary gameObjects_;
	GizmosPainterContainer gizmosPainters_;
	GameObjectSequence cullingUpdateSequence_;
	GameObjectSequence renderingUpdateSequence_;

	Environment environment_;

	ref_ptr<GameObject> root_;
	GameObjectImporter* importer_;
};
