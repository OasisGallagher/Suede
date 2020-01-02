#include "mesh.h"
#include "renderer.h"
#include "particlesystem.h"
#include "componentinternal.h"

class RenderState;

class RendererInternal : public ComponentInternal {
public:
	RendererInternal(ObjectType type);
	~RendererInternal();

public:
	virtual void UpdateMaterialProperties() {}

	int GetUpdateStrategy() { return UpdateStrategyNone; }

	void AddMaterial(Material* material) { materials_.push_back(material); }
	Material* GetMaterial(uint index) { return materials_[index].get(); }
	void SetMaterial(uint index, Material* value) { materials_[index] = value; }
	void RemoveMaterial(Material* material);
	void RemoveMaterialAt(uint index);
	uint GetMaterialCount() { return materials_.size(); }

	uint GetRenderQueue() { return queue_; }
	void SetRenderQueue(uint value) { queue_ = value; }

	virtual const Bounds& GetBounds() { UpdateBounds(); return bounds_; }

protected:
	virtual void UpdateBounds() = 0;

protected:
	Bounds bounds_;

private:
	uint queue_;
	std::vector<ref_ptr<Material>> materials_;
};

class MeshRendererInternal : public RendererInternal {
public:
	MeshRendererInternal() : RendererInternal(ObjectType::MeshRenderer) {}

protected:
	virtual void UpdateBounds();
};

class SkinnedMeshRendererInternal :public RendererInternal {
public:
	SkinnedMeshRendererInternal() : RendererInternal(ObjectType::SkinnedMeshRenderer) {}

public:
	virtual void UpdateMaterialProperties();

public:
	void SetSkeleton(Skeleton* value) { skeleton_ = value; }

protected:
	virtual void UpdateBounds();

private:
	ref_ptr<Skeleton> skeleton_;
};

class ParticleRendererInternal : public RendererInternal {
public:
	ParticleRendererInternal();

protected:
	virtual void UpdateBounds();
};
