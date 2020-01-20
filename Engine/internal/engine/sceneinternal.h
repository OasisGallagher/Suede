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
	~SceneInternal();

public:
	void GetDecals(std::vector<Decal>& container);
	Environment* GetEnvironment() { return &environment_; }

	GameObject* GetGameObject(uint id);
	void DestroyGameObject(uint id);
	void DestroyGameObject(GameObject* go);
	std::vector<GameObject*> GetGameObjectsOfComponent(suede_guid guid);
	Transform* GetRootTransform() { return root_->GetTransform(); }
	void Import(const std::string& path, std::function<void(GameObject*)> callback);

public:
	void Awake();
	void Update(float deltaTime);

	void CullingUpdate(float deltaTime);
	void OnDestroy();

private:
	void UpdateDecals();
	void SortComponents();
	void UpdateDestroyedGameObjects();
	void CullingUpdateGameObjects(float deltaTime);
	void RenderingUpdateGameObjects(float deltaTime);
	void AddGameObject(ref_ptr<GameObject> go);
	void RemoveGameObject(GameObject* go);
	void RemoveGameObjectFromSequence(GameObject* go);
	void ManageGameObjectUpdateSequence(GameObject* go, ComponentEventType type);
	void DestroyGameObjectRecursively(Transform* root);

	void OnTransformAttached(Transform* transform, bool attached);
	void OnGameObjectComponentChanged(ref_ptr<GameObject> go, ComponentEventType type, ref_ptr<Component> component);

	template <class Container>
	void ManageGameObjectComponents(Container& container, Component* component, ComponentEventType type);

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

	enum : char { FramesDestroyedGameObjectAlive = 2 };
	std::vector<std::pair<ref_ptr<GameObject>, char>> destroyed_;

	ref_ptr<GameObject> root_;
	GameObjectImporter* importer_;
};
