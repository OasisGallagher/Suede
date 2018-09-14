#pragma once
#include "mesh.h"
#include "animation.h"
#include "gameobject.h"
#include "internal/base/objectinternal.h"

class GameObjectInternal : virtual public IGameObject, public ObjectInternal {
	DEFINE_FACTORY_METHOD(GameObject)

public:
	GameObjectInternal();
	~GameObjectInternal();

protected:
	GameObjectInternal(ObjectType gameObjectType);

public:
	virtual bool GetActive() const { return active_; }

	virtual void SetActiveSelf(bool value);
	virtual bool GetActiveSelf() const { return activeSelf_; }

	virtual int GetUpdateStrategy();

	virtual const std::string& GetTag() const { return tag_; }
	virtual bool SetTag(const std::string& value);

	virtual std::string GetName() const { return name_; }
	virtual void SetName(const std::string& value);

	virtual void CullingUpdate();
	virtual void RenderingUpdate();

	virtual Transform GetTransform();

	virtual const Bounds& GetBounds();
	virtual void RecalculateBounds(int flags = RecalculateBoundsFlagsAll);

	virtual void RecalculateUpdateStrategy();

private:
	virtual Component AddComponentHelper(suede_guid type);
	virtual Component AddComponentHelper(Component component);

	virtual Component GetComponentHelper(suede_guid type);
	virtual std::vector<Component> GetComponentsHelper(suede_guid type);

private:
	void CalculateSelfWorldBounds();
	void CalculateBonesWorldBounds();

	void CalculateHierarchyBounds();
	void CalculateHierarchyMeshBounds();

	void DirtyParentBounds();
	void DirtyChildrenBoundses();

	int GetHierarchyUpdateStrategy(GameObject root);
	bool RecalculateHierarchyUpdateStrategy();

	void SetActive(bool value);
	void UpdateChildrenActive(GameObject parent);

	template <class T>
	void FireWorldEvent(bool attachedToSceneOnly);

	bool CheckComponentDuplicate(suede_guid type);

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
inline void GameObjectInternal::FireWorldEvent(bool attachedToSceneOnly) {
	if (!attachedToSceneOnly || GetTransform()->IsAttachedToScene()) {
		T e = NewWorldEvent<T>();
		e->go = SharedThis();
		World::instance()->FireEvent(e);
	}
}
