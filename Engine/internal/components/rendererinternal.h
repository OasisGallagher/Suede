#include "mesh.h"
#include "renderer.h"
#include "particlesystem.h"
#include "componentinternal.h"

class RenderState;

class RendererInternal : public ComponentInternal {
public:
	RendererInternal(ObjectType type);

public:
	virtual void UpdateMaterialProperties() {}

	int GetUpdateStrategy() { return UpdateStrategyNone; }

	void AddSharedMaterial(Material* material) { materials_.push_back({ nullptr, material }); }

	Material* GetMaterial(uint index);
	Material* GetSharedMaterial(uint index) { return materials_[index].sharedMaterial.get(); }

	bool IsMaterialInstantiated(uint index) { return !!materials_[index].material; }
	void SetMaterial(uint index, Material* value) { materials_[index] = { nullptr, value }; }
	void RemoveMaterialAt(uint index);
	uint GetMaterialCount() { return materials_.size(); }

	uint GetRenderQueue() { return queue_; }
	void SetRenderQueue(uint value) { queue_ = value; }

	virtual const Bounds& GetBounds();

protected:
	virtual void UpdateBounds() = 0;

protected:
	Bounds bounds_;

private:
	uint queue_;

	struct Pair {
		ref_ptr<Material> material;
		ref_ptr<Material> sharedMaterial;
	};

	std::vector<Pair> materials_;
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
