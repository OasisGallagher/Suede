#pragma once
#include "mesh.h"
#include "animation.h"
#include "gameobject.h"
#include "internal/base/objectinternal.h"

class GameObjectInternal : public ObjectInternal {
public:
	GameObjectInternal(Scene* scene, Tags* tags, const char* name = "");
	~GameObjectInternal();

public:
	Scene* GetScene() { return scene_; }
	bool GetActive() const { return active_; }

	void SetActiveSelf(GameObject* self, bool value);
	bool GetActiveSelf() const { return activeSelf_; }

	int GetUpdateStrategy(GameObject* self);
	void SendMessage(int messageID, void* parameter);

	const std::string& GetTag() const { return tag_; }
	bool SetTag(GameObject* self, const std::string& value);

	void Update(float deltaTime);
	void CullingUpdate(float deltaTime);

	Transform* GetTransform();

	const Bounds& GetBounds();
	void RecalculateBounds(int flags = RecalculateBoundsFlagsAll);

	void RecalculateUpdateStrategy(GameObject* self);

protected:
	virtual void OnNameChanged(Object* self);

public:
	template <class T>
	Component* AddComponent(GameObject* self, T key);

	template <class T>
	Component* GetComponent(T key);

	template <class T> T* GetComponent();

	template <class T>
	std::vector<Component*> GetComponents(T key);

private:
	void CalculateBonesWorldBounds();
	void CalculateSelfWorldBounds(const Bounds& bounds);

	void CalculateHierarchyBounds();
	void CalculateHierarchyMeshBounds();

	void DirtyParentBounds();
	void DirtyChildrenBoundses();

	Component* ActivateComponent(GameObject* self, Component* component);

	int GetHierarchyUpdateStrategy(GameObject* root);
	bool RecalculateHierarchyUpdateStrategy(GameObject* self);

	void SetActive(GameObject* self, bool value);
	void UpdateChildrenActive(GameObject* parent);

	template <class Event, class... Args>
	void RaiseGameObjectEvent(Event& e, bool attachedToSceneOnly, Args... args);

	template <class T>
	bool CheckComponentDuplicate(T key);

private:
	Tags* tags_;
	Scene* scene_;

	bool active_;
	bool activeSelf_;

	std::string tag_;

	std::vector<ref_ptr<Component>> components_;

	uint updateStrategy_;
	bool updateStrategyDirty_;

	Bounds worldBounds_;
	bool boundsDirty_;
};

template <class Event, class... Args>
inline void GameObjectInternal::RaiseGameObjectEvent(Event& e, bool attachedToSceneOnly, Args... args) {
	if (!attachedToSceneOnly || GetTransform()->IsAttachedToScene()) {
		e.delay_raise(args...);
	}
}

template <class T>
inline bool GameObjectInternal::CheckComponentDuplicate(T key) {
	if (GetComponent(key)) {
		Debug::LogError("component with type %s already exist", std::to_string(key).c_str());
		return false;
	}

	return true;
}

template <class T>
inline Component* GameObjectInternal::AddComponent(GameObject* self, T key) {
	Component* component = (Component*)Factory::Create(key);
	if (component->AllowMultiple() || CheckComponentDuplicate(key)) {
		return ActivateComponent(self, component);
	}

	return nullptr;
}

template <>
inline Component* GameObjectInternal::AddComponent(GameObject* self, Component* key) {
	if (key->AllowMultiple() || CheckComponentDuplicate(key->GetComponentInstanceGUID())) {
		return ActivateComponent(self, key);
	}

	return nullptr;
}

template <class T>
inline T* GameObjectInternal::GetComponent() {
	return (T*)GetComponent(T::GetComponentGUID());
}

template <class T>
Component* GameObjectInternal::GetComponent(T key) {
	for (ref_ptr<Component>& component : components_) {
		if (component->IsComponentType(key)) {
			return component.get();
		}
	}

	return nullptr;
}

template <class T>
std::vector<Component*> GameObjectInternal::GetComponents(T key) {
	std::vector<Component*> container;
	for (ref_ptr<Component>& component : components_) {
		if (component->IsComponentType(key)) {
			container.push_back(component.get());
		}
	}

	return container;
}
