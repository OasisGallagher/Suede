#include <wrappers/gl.h>

#include "mesh.h"
#include "renderer.h"
#include "particlesystem.h"
#include "internal/base/objectinternal.h"

class RenderState;

class RendererInternal : virtual public IRenderer, public ObjectInternal {
public:
	RendererInternal(ObjectType type);
	~RendererInternal();

public:
	virtual void RenderSprite(Sprite sprite);
	virtual void RenderMesh(Mesh mesh, Material material);

	virtual void AddMaterial(Material material) { materials_.push_back(material); }
	virtual Material GetMaterial(uint index) { return materials_[index]; }
	virtual void SetMaterial(uint index, Material value) { materials_[index] = value; }
	virtual void RemoveMaterial(Material material);
	virtual void RemoveMaterialAt(uint index);
	virtual uint GetMaterialCount() { return materials_.size(); }

	virtual uint GetRenderQueue() { return queue_; }
	virtual void SetRenderQueue(uint value) { queue_ = value; }

protected:
	virtual void UpdateMaterial(Sprite sprite);
	virtual void DrawCall(SubMesh subMesh, MeshTopology topology);

protected:
	GLenum TopologyToGLEnum(MeshTopology topology);

private:
	virtual void RenderMesh(Mesh mesh);

	void RenderMesh(Mesh mesh, Material material, int pass);
	void RenderSubMesh(Mesh mesh, int subMeshIndex, Material material, int pass);

private:
	uint queue_;
	std::vector<Material> materials_;
};

class MeshRendererInternal : public IMeshRenderer, public RendererInternal {
	DEFINE_FACTORY_METHOD(MeshRenderer)

public:
	MeshRendererInternal() : RendererInternal(ObjectTypeMeshRenderer) {}
};

class SkinnedMeshRendererInternal : public ISkinnedMeshRenderer, public RendererInternal {
	DEFINE_FACTORY_METHOD(SkinnedMeshRenderer)

public:
	SkinnedMeshRendererInternal() : RendererInternal(ObjectTypeSkinnedMeshRenderer) {}

public:
	virtual void UpdateMaterial(Sprite sprite);
	virtual void SetSkeleton(Skeleton value) { skeleton_ = value; }

private:
	Skeleton skeleton_;
};

class ParticleRendererInternal : public IParticleRenderer, public RendererInternal {
	DEFINE_FACTORY_METHOD(ParticleRenderer)

public:
	ParticleRendererInternal();

public:
	virtual void RenderSprite(Sprite sprite);
	virtual void AddMaterial(Material material);

protected:
	virtual void DrawCall(SubMesh subMesh, MeshTopology topology);

private:
	uint particleCount_;
};
