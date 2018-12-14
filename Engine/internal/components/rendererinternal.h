#include "mesh.h"
#include "renderer.h"
#include "../api/gl.h"
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

	void AddMaterial(Material material) { materials_.push_back(material); }
	Material GetMaterial(uint index) { return materials_[index]; }
	IRenderer::Enumerable GetMaterials() { return IRenderer::Enumerable(materials_.begin(), materials_.end()); }
	void SetMaterial(uint index, Material value) { materials_[index] = value; }
	void RemoveMaterial(Material material);
	void RemoveMaterialAt(uint index);
	uint GetMaterialCount() { return materials_.size(); }

	uint GetRenderQueue() { return queue_; }
	void SetRenderQueue(uint value) { queue_ = value; }

private:
	uint queue_;
	std::vector<Material> materials_;
};

class MeshRendererInternal : public RendererInternal {
public:
	MeshRendererInternal() : RendererInternal(ObjectType::MeshRenderer) {}
};

class SkinnedMeshRendererInternal :public RendererInternal {
public:
	SkinnedMeshRendererInternal() : RendererInternal(ObjectType::SkinnedMeshRenderer) {}

public:
	virtual void UpdateMaterialProperties();

public:
	void SetSkeleton(Skeleton value) { skeleton_ = value; }

private:
	Skeleton skeleton_;
};

class ParticleRendererInternal : public RendererInternal {
public:
	ParticleRendererInternal();
};
