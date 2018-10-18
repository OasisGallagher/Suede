#pragma once
#include "mesh.h"
#include "animation.h"
#include "gameobject.h"
#include "internal/base/objectinternal.h"

class GameObjectInternal : public ObjectInternal {
public:
	GameObjectInternal();
	~GameObjectInternal();

protected:
	GameObjectInternal(ObjectType type);

public:
	bool GetActive() const { return active_; }

	void SetActiveSelf(GameObject self, bool value);
	bool GetActiveSelf() const { return activeSelf_; }

	int GetUpdateStrategy(GameObject self);

	const std::string& GetTag() const { return tag_; }
	bool SetTag(GameObject self, const std::string& value);

	std::string GetName() const { return name_; }
	void SetName(GameObject self, const std::string& value);

	void CullingUpdate();
	void RenderingUpdate();

	Transform GetTransform();

	const Bounds& GetBounds();
	void RecalculateBounds(int flags = RecalculateBoundsFlagsAll);

	void RecalculateUpdateStrategy(GameObject self);

public:
	template <class T>
	Component AddComponent(GameObject self, const T& key);
	Component AddComponent(GameObject self, Component component);


	template <class T>
	Component GetComponent(const T& key);

	template <class T>
	std::vector<Component> GetComponents(const T& key);

private:
	void CalculateBonesWorldBounds();
	void CalculateSelfWorldBounds(Mesh mesh);

	void CalculateHierarchyBounds();
	void CalculateHierarchyMeshBounds();

	void DirtyParentBounds();
	void DirtyChildrenBoundses();

	Component ActivateComponent(GameObject self, Component component);

	int GetHierarchyUpdateStrategy(GameObject root);
	bool RecalculateHierarchyUpdateStrategy(GameObject self);

	void SetActive(GameObject self, bool value);
	void UpdateChildrenActive(GameObject parent);

	template <class T>
	void FireWorldEvent(GameObject self, bool attachedToSceneOnly);

	template <class T>
	bool CheckComponentDuplicate(const T& key);

private:
	bool active_;
	bool activeSelf_;

	std::string tag_;
	std::string name_;

	std::vector<Component> components_;

	uint updateStrategy_;
	bool updateStrategyDirty_;

	uint frameCullingUpdate_;

	Bounds worldBounds_;
	// is world space dirty.
	bool boundsDirty_;
};

template <class T>
inline void GameObjectInternal::FireWorldEvent(GameObject self, bool attachedToSceneOnly) {
	if (!attachedToSceneOnly || GetTransform()->IsAttachedToScene()) {
		T e = NewWorldEvent<T>();
		e->go = self;
		World::instance()->FireEvent(e);
	}
}

template <class T>
bool GameObjectInternal::CheckComponentDuplicate(const T& key) {
	if (GetComponent(key)) {
		Debug::LogError("component with type %s already exist", std::to_string(key).c_str());
		return false;
	}

	return true;
}

template <class T>
Component GameObjectInternal::AddComponent(GameObject self, const T& key) {
	if (!CheckComponentDuplicate(key)) {
		return nullptr;
	}

	Component component = suede_dynamic_cast<Component>(Factory::Create(key));
	return ActivateComponent(self, component);
}

template <class T>
Component GameObjectInternal::GetComponent(const T& key) {
	for (Component component : components_) {
		if (component->IsComponentType(key)) {
			return component;
		}
	}

	return nullptr;
}

template <class T>
std::vector<Component> GameObjectInternal::GetComponents(const T& key) {
	std::vector<Component> container;
	for (Component component : components_) {
		if (component->IsComponentType(key)) {
			container.push_back(component);
		}
	}

	return container;
}
