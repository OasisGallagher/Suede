#pragma once
#include "mesh.h"
#include "entity.h"
#include "animation.h"
#include "internal/base/objectinternal.h"

class EntityInternal : virtual public IEntity, public ObjectInternal {
	DEFINE_FACTORY_METHOD(Entity)

public:
	EntityInternal();

public:
	virtual void SetStatus(EntityStatus value) { status_ = value; }
	virtual EntityStatus GetStatus() const { return status_; }

	virtual bool GetActive() const { return active_; }

	virtual void SetActiveSelf(bool value);
	virtual bool GetActiveSelf() const { return activeSelf_; }

	virtual const std::string& GetTag() const { return tag_; }
	virtual bool SetTag(const std::string& value);

	virtual std::string GetName() const { return name_; }
	virtual void SetName(const std::string& value);

	virtual void Update();

	virtual void SetTransform(Transform value);
	virtual Transform GetTransform() const { return transform_; }

	virtual void SetAnimation(Animation value);
	virtual Animation GetAnimation() { return animation_; }

	virtual const Bounds& GetBounds();
	virtual void RecalculateBounds();

	virtual void SetMesh(Mesh value);
	virtual Mesh GetMesh() { return mesh_; }

	virtual void SetRenderer(Renderer value);
	virtual Renderer GetRenderer() { return renderer_; }

protected:
	EntityInternal(ObjectType entityType);

private:
	void CalculateSelfWorldBounds();
	void CalculateBonesWorldBounds();

	void CalculateHierarchyBounds();
	void CalculateHierarchyMeshBounds();

	void DirtyParentBounds();
	void DirtyChildrenBoundses();

	void SetActive(bool value);
	void UpdateChildrenActive(Entity parent);

private:
	static const char* EntityTypeToString(ObjectType type);

private:
	bool active_;
	bool activeSelf_;
	EntityStatus status_;

	std::string tag_;
	std::string name_;

	Transform transform_;

	Mesh mesh_;

	Bounds worldBounds_;
	// is world space dirty.
	bool boundsDirty_;

	Renderer renderer_;
	Animation animation_;
};
