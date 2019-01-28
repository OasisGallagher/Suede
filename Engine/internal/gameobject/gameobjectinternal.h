﻿#pragma once
#include "mesh.h"
#include "animation.h"
#include "gameobject.h"
#include "internal/base/objectinternal.h"

class GameObjectInternal : public ObjectInternal {
	SUEDE_DECLARE_SELF_TYPE(IGameObject)

public:
	GameObjectInternal(IGameObject* self);
	~GameObjectInternal();

protected:
	GameObjectInternal(IGameObject* self, ObjectType type);

public:
	bool GetActive() const { return active_; }

	void SetActiveSelf(bool value);
	bool GetActiveSelf() const { return activeSelf_; }

	void SetLayer(uint value) { layer_ = value; }
	uint GetLayer() const { return layer_; }

	int GetUpdateStrategy();
	void SendMessage(int messageID, void* parameter);

	const std::string& GetTag() const { return tag_; }
	bool SetTag(const std::string& value);

	void Update();
	void OnPreRender();
	void OnPostRender();
	void CullingUpdate();

	Transform GetTransform();

	const Bounds& GetBounds();
	void RecalculateBounds(int flags = RecalculateBoundsFlagsAll);

	void RecalculateUpdateStrategy();

protected:
	virtual void OnNameChanged();

public:
	template <class T>
	Component AddComponent(T key);

	template <class T>
	Component GetComponent(T key);

	template <class T> T GetComponent();

	template <class T>
	std::vector<Component> GetComponents(T key);

private:
	void CalculateBonesWorldBounds();
	void CalculateSelfWorldBounds(const Bounds& bounds);

	void CalculateHierarchyBounds();
	void CalculateHierarchyMeshBounds();

	void DirtyParentBounds();
	void DirtyChildrenBoundses();

	Component ActivateComponent(Component component);

	void SetActive(bool value);
	void UpdateChildrenActive(GameObject parent);

	template <class T, class... Args>
	void FireWorldEvent(bool attachedToSceneOnly, bool immediate, Args... args);

	template <class T>
	bool CheckComponentDuplicate(T key);

private:
	bool active_;
	bool activeSelf_;

	uint layer_;
	std::string tag_;

	std::vector<Component> components_;

	uint updateStrategy_;
	bool updateStrategyDirty_;

	// ensure CullingUpdate to be called once per frame.
	uint frameCullingUpdate_;

	Bounds worldBounds_;
	bool boundsDirty_;
};

template <class T, class... Args>
inline void GameObjectInternal::FireWorldEvent(bool attachedToSceneOnly, bool immediate, Args... args) {
	if (!attachedToSceneOnly || GetTransform()->IsAttachedToScene()) {
		if (immediate) { World::FireEventImmediate(T(_suede_self(), args...)); }
		else { World::FireEvent(new T(_suede_self(), args...)); }
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
inline Component GameObjectInternal::AddComponent(T key) {
	Component component = suede_dynamic_cast<Component>(Factory::Create(key));
	if (component->AllowMultiple() || CheckComponentDuplicate(key)) {
		return ActivateComponent(component);
	}

	return nullptr;
}

template <>
inline Component GameObjectInternal::AddComponent(Component key) {
	if (key->AllowMultiple() || CheckComponentDuplicate(key->GetComponentInstanceGUID())) {
		return ActivateComponent(key);
	}

	return nullptr;
}

template <class T>
inline T GameObjectInternal::GetComponent() {
	return suede_dynamic_cast<T>(GetComponent(T::element_type::GetComponentGUID()));
}

template <class T>
Component GameObjectInternal::GetComponent(T key) {
	for (Component component : components_) {
		if (component->IsComponentType(key)) {
			return component;
		}
	}

	return nullptr;
}

template <class T>
std::vector<Component> GameObjectInternal::GetComponents(T key) {
	std::vector<Component> container;
	for (Component component : components_) {
		if (component->IsComponentType(key)) {
			container.push_back(component);
		}
	}

	return container;
}
