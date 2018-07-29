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
	virtual void RecalculateBounds(int flags = RecalculateBoundsFlagsAll);

	virtual void SetMesh(Mesh value);
	virtual Mesh GetMesh() { return mesh_; }

	virtual void SetParticleSystem(ParticleSystem value);
	virtual ParticleSystem GetParticleSystem() { return particleSystem_; }

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

	Mesh mesh_;

	Bounds worldBounds_;
	// is world space dirty.
	bool boundsDirty_;

	Renderer renderer_;
	Animation animation_;
	ParticleSystem particleSystem_;
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
}

template <class T>
inline void EntityInternal::FireWorldEvent(bool attachedToSceneOnly) {
	if (!attachedToSceneOnly || transform_->IsAttachedToScene()) {
		T e = NewWorldEvent<T>();
		e->entity = SharedThis();
		World::get()->FireEvent(e);
	}
}
