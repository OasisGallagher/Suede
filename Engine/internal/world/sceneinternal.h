#pragma once
#include <map>

#include "world.h"
#include "light.h"
#include "camera.h"
#include "projector.h"
#include "gameobject.h"
#include "gizmospainter.h"
#include "containers/sortedvector.h"

class DecalCreater;

class SceneInternal {
public:
	SceneInternal();
	~SceneInternal();

public:
	void Update();
	void CullingUpdate();

	void GetDecals(std::vector<Decal>& container);
	GameObject* GetGameObject(uint id);
	void DestroyGameObject(uint id);
	void DestroyGameObject(GameObject* go);
	std::vector<GameObject*> GetGameObjectsOfComponent(suede_guid guid);
	void WalkGameObjectHierarchy(std::function<WalkCommand(GameObject*)> walker);
	Transform* GetRootTransform() { return root_->GetTransform(); }

private:
	void UpdateDecals();
	void CullingUpdateGameObjects();
	void RenderingUpdateGameObjects();
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
	struct LightComparer { bool operator() (const ref_ptr<Light>& lhs, const ref_ptr<Light>& rhs) const; };
	struct CameraComparer { bool operator() (const ref_ptr<Camera>& lhs, const ref_ptr<Camera>& rhs) const; };
	struct ProjectorComparer { bool operator() (const ref_ptr<Projector>& lhs, const ref_ptr<Projector>& rhs) const; };

	typedef sorted_vector<ref_ptr<GizmosPainter>> GizmosPainterContainer;

	typedef sorted_vector<GameObject*> GameObjectSequence;
	typedef std::map<uint, ref_ptr<GameObject>> GameObjectDictionary;
	typedef std::set<ref_ptr<Light>, LightComparer> LightContainer;
	typedef sorted_vector<ref_ptr<Camera>, CameraComparer> CameraContainer;
	typedef std::set<ref_ptr<Projector>, ProjectorComparer> ProjectorContainer;

private:
	LightContainer lights_;
	CameraContainer cameras_;
	DecalCreater* decalCreater_;
	ProjectorContainer projectors_;
	GameObjectDictionary gameObjects_;
	GizmosPainterContainer gizmosPainters_;
	GameObjectSequence cullingUpdateSequence_;
	GameObjectSequence renderingUpdateSequence_;

	ref_ptr<GameObject> root_;
};

template <class Container>
void SceneInternal::ManageGameObjectComponents(Container& container, Component* component, ComponentEventType state) {
	typedef typename Container::value_type V;
	typedef typename V::element_type T;

	if (component->IsComponentType(T::GetComponentGUID())) {
		T* target = (T*)component;
		if (state == ComponentEventType::Added) {
			container.insert(container.end(), ref_ptr<T>(target));
		}
		else if (state == ComponentEventType::Removed) {
			container.erase(ref_ptr<T>(target));
		}
	}
}
