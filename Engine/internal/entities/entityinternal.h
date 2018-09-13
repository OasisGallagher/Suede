#pragma once
#include "mesh.h"
#include "entity.h"
#include "animation.h"
#include "internal/base/objectinternal.h"

class EntityInternal : virtual public IEntity, public ObjectInternal {
	DEFINE_FACTORY_METHOD(Entity)

public:
	EntityInternal();
	~EntityInternal();

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

	virtual void SetTransform(Transform value);
	virtual Transform GetTransform() const { return transform_; }

	virtual const Bounds& GetBounds();
	virtual void RecalculateBounds(int flags = RecalculateBoundsFlagsAll);

	virtual void RecalculateUpdateStrategy();

protected:
	EntityInternal(ObjectType entityType);

private:
	void CalculateSelfWorldBounds();
	void CalculateBonesWorldBounds();

	void CalculateHierarchyBounds();
	void CalculateHierarchyMeshBounds();

	void DirtyParentBounds();
	void DirtyChildrenBoundses();

	int GetHierarchyUpdateStrategy(Entity root);
	bool RecalculateHierarchyUpdateStrategy();

	void SetActive(bool value);
	void UpdateChildrenActive(Entity parent);

	template <class T>
	void SetComponent(T& ref, T value);

	template <class T>
	void FireWorldEvent(bool attachedToSceneOnly);

private:
	static const char* EntityTypeToString(ObjectType type);

private:
	bool active_;
	bool activeSelf_;

	std::string tag_;
	std::string name_;

	Transform transform_;

	uint updateStrategy_;
	bool updateStrategyDirty_;

	uint frameCullingUpdate_;

	Bounds worldBounds_;
	// is world space dirty.
	bool boundsDirty_;
};

template <class T>
inline void EntityInternal::SetComponent(T& ref, T value) {
	if (ref == value) { return; }

	if (ref) {
		ref->SetEntity(nullptr);
	}

	if (ref = value) {
		ref->SetEntity(SharedThis());
	}

	if (!value || value->GetUpdateStrategy() != UpdateStrategyNone) {
		RecalculateUpdateStrategy();
	}
}

template <class T>
inline void EntityInternal::FireWorldEvent(bool attachedToSceneOnly) {
	if (!attachedToSceneOnly || transform_->IsAttachedToScene()) {
		T e = NewWorldEvent<T>();
		e->entity = SharedThis();
		World::instance()->FireEvent(e);
	}
}
