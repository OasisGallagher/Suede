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

	virtual void SetAnimation(Animation value) { animation_ = value; }
	virtual Animation GetAnimation() { return animation_; }

	// TODO: animation may change bounds.
	virtual const Bounds& GetBounds();

	virtual void RecalculateBounds();
	virtual void SetMeshBounds(const Bounds& value);

	virtual void SetMesh(Mesh value) { mesh_ = value; }
	virtual Mesh GetMesh() { return mesh_; }

	virtual void SetRenderer(Renderer value) { renderer_ = value; }
	virtual Renderer GetRenderer() { return renderer_; }

protected:
	EntityInternal(ObjectType entityType);

private:
	void CalculateMeshBounds();
	void CalculateBonesBounds();

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

	std::string tag_;
	std::string name_;

	Transform transform_;

	Mesh mesh_;

	mutable Bounds bounds_;
	mutable bool boundsDirty_;

	// initial bounds in local space.
	Bounds meshBounds;

	Renderer renderer_;
	Animation animation_;
};
