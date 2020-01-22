#pragma once
#include "mesh.h"
#include "animation.h"
#include "gameobject.h"
#include "internal/base/objectinternal.h"

class Tags;
class Context;
class GameObjectInternal : public ObjectInternal {
public:
	GameObjectInternal(Context* context, Scene* scene, Tags* tags, const char* name = "");

public:
	Scene* GetScene() { return scene_; }
	bool GetActive() const { return active_; }

	void SetLayer(int value, bool recursivly);
	int GetLayer() const { return layer_; }

	void SetActiveSelf(GameObject* self, bool value);
	bool GetActiveSelf() const { return activeSelf_; }

	int GetUpdateStrategy(GameObject* self);
	void SendMessage(int messageID, void* parameter);

	const std::string& GetTag() const { return tag_; }
	bool SetTag(GameObject* self, const std::string& value);

	void Update(float deltaTime);
	void CullingUpdate(float deltaTime);

	Transform* GetTransform();

	void RecalculateUpdateStrategy(GameObject* self);

public:
	static event<ref_ptr<GameObject>> created;
	static event<ref_ptr<GameObject>, ComponentEventType, ref_ptr<Component>> componentChanged;

public:
	template <class T>
	Component* AddComponent(GameObject* self, T key);

	template <class T>
	Component* GetComponent(T key);

	template <class T>
	std::vector<Component*> GetComponentsInChildren(T key);

	template <class T> T* GetComponent();

	template <class T>
	std::vector<Component*> GetComponents(T key);

private:
	Component* ActivateComponent(GameObject* self, Component* component);

	int GetHierarchyUpdateStrategy(GameObject* root);
	bool RecalculateHierarchyUpdateStrategy(GameObject* self);

	void SetActive(GameObject* self, bool value);
	void UpdateChildrenActive(GameObject* parent);

	template <class T>
	bool CheckComponentDuplicate(T key);

private:
	Tags* tags_;
	Scene* scene_;
	Context* context_;

	int layer_ = 0;

	bool active_ = true;
	bool activeSelf_ = true;

	std::string tag_;

	std::vector<ref_ptr<Component>> components_;

	uint updateStrategy_ = UpdateStrategyNone;
	bool updateStrategyDirty_ = true;
};

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

template <class T>
std::vector<Component*> GameObjectInternal::GetComponentsInChildren(T key) {
	std::vector<Component*> container = GetComponents(key);
	Transform* transform = GetTransform();
	for (int i = 0; i < transform->GetChildCount(); ++i) {
		GameObject* child = transform->GetChildAt(i)->GetGameObject();
		std::vector<Component*> components = child->GetComponentsInChildren(key);
		container.insert(container.end(), components.begin(), components.end());
	}

	return container;
}
