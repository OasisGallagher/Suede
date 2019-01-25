#include "mesh.h"
#include "renderer.h"
#include "../api/gl.h"
#include "particlesystem.h"
#include "componentinternal.h"

class RenderState;

class RendererInternal : public ComponentInternal {
	SUEDE_DECLARE_SELF_TYPE(IRenderer)

public:
	RendererInternal(IRenderer* self, ObjectType type);
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
	SUEDE_DECLARE_SELF_TYPE(IMeshRenderer)

public:
	MeshRendererInternal(IMeshRenderer* self) : RendererInternal(self, ObjectType::MeshRenderer) {}
};

class SkinnedMeshRendererInternal :public RendererInternal {
	SUEDE_DECLARE_SELF_TYPE(ISkinnedMeshRenderer)

public:
	SkinnedMeshRendererInternal(ISkinnedMeshRenderer* self) : RendererInternal(self, ObjectType::SkinnedMeshRenderer) {}

public:
	virtual void UpdateMaterialProperties();

public:
	void SetSkeleton(Skeleton value) { skeleton_ = value; }

private:
	Skeleton skeleton_;
};

class ParticleRendererInternal : public RendererInternal {
	SUEDE_DECLARE_SELF_TYPE(IParticleRenderer)

public:
	ParticleRendererInternal(IParticleRenderer* self);
};
