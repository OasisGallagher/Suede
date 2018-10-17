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
	Component AddComponent(GameObject self, suede_guid guid);
	Component AddComponent(GameObject self, Component component);

	Component GetComponent(suede_guid guid);
	std::vector<Component> GetComponents(suede_guid guid);

private:
	void CalculateBonesWorldBounds();
	void CalculateSelfWorldBounds(Mesh mesh);

	void CalculateHierarchyBounds();
	void CalculateHierarchyMeshBounds();

	void DirtyParentBounds();
	void DirtyChildrenBoundses();

	int GetHierarchyUpdateStrategy(GameObject root);
	bool RecalculateHierarchyUpdateStrategy(GameObject self);

	void SetActive(GameObject self, bool value);
	void UpdateChildrenActive(GameObject parent);

	template <class T>
	void FireWorldEvent(GameObject self, bool attachedToSceneOnly);

	bool CheckComponentDuplicate(suede_guid guid);

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
