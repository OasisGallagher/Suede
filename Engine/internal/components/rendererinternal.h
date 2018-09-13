#include "mesh.h"
#include "renderer.h"
#include "../api/gl.h"
#include "particlesystem.h"
#include "componentinternal.h"

class RenderState;

class RendererInternal : virtual public IRenderer, public ComponentInternal {
public:
	RendererInternal(ObjectType type);
	~RendererInternal();

public:
	virtual void UpdateMaterialProperties() {}

	virtual int GetUpdateStrategy() { return UpdateStrategyNone; }

	virtual void AddMaterial(Material material) { materials_.push_back(material); }
	virtual Material GetMaterial(uint index) { return materials_[index]; }
	virtual Enumerable GetMaterials() { return Enumerable(materials_.begin(), materials_.end()); }
	virtual void SetMaterial(uint index, Material value) { materials_[index] = value; }
	virtual void RemoveMaterial(Material material);
	virtual void RemoveMaterialAt(uint index);
	virtual uint GetMaterialCount() { return materials_.size(); }

	virtual uint GetRenderQueue() { return queue_; }
	virtual void SetRenderQueue(uint value) { queue_ = value; }

private:
	uint queue_;
	std::vector<Material> materials_;
};

class MeshRendererInternal : public IMeshRenderer, public RendererInternal {
public:
	MeshRendererInternal() : RendererInternal(ObjectType::MeshRenderer) {}
};

class SkinnedMeshRendererInternal : public ISkinnedMeshRenderer, public RendererInternal {
public:
	SkinnedMeshRendererInternal() : RendererInternal(ObjectType::SkinnedMeshRenderer) {}

public:
	virtual void UpdateMaterialProperties();
	virtual void SetSkeleton(Skeleton value) { skeleton_ = value; }

private:
	Skeleton skeleton_;
};

class ParticleRendererInternal : public IParticleRenderer, public RendererInternal {
public:
	ParticleRendererInternal();
};
