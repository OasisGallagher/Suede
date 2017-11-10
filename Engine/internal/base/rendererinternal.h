#include "surface.h"
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
	virtual void RenderSurface(Surface surface);

	virtual void AddMaterial(Material material) { materials_.push_back(material); }
	virtual Material GetMaterial(int index) { return materials_[index]; }
	virtual void SetMaterial(int index, Material value) { materials_[index] = value; }
	virtual int GetMaterialCount() { return materials_.size(); }

	virtual int GetRenderQueue() { return queue_; }
	virtual void SetRenderQueue(int value) { queue_ = value; }

protected:
	virtual void DrawCall(Mesh mesh);

protected:
	GLenum TopologyToGLEnum(MeshTopology topology);

private:
	int queue_;
	std::vector<Material> materials_;
};

class SurfaceRendererInternal : public ISurfaceRenderer, public RendererInternal {
	DEFINE_FACTORY_METHOD(SurfaceRenderer)

public:
	SurfaceRendererInternal() : RendererInternal(ObjectTypeSurfaceRenderer) {}
};

class SkinnedSurfaceRendererInternal : public ISkinnedSurfaceRenderer, public RendererInternal {
	DEFINE_FACTORY_METHOD(SkinnedSurfaceRenderer)

public:
	SkinnedSurfaceRendererInternal() : RendererInternal(ObjectTypeSkinnedSurfaceRenderer) {}

public:
	virtual void RenderSurface(Surface surface);
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
	virtual void DrawCall(Mesh mesh);

private:
	unsigned particleCount_;
};
