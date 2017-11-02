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

	virtual void SetRenderState(RenderStateType type, int parameter0, int parameter1);

protected:
	virtual void DrawCall(Mesh mesh);

	GLenum PrimaryTypeToGLEnum(PrimaryType type);

private:
	void DrawSurface(Surface surface);
	void DrawMesh(Mesh mesh, Material material);

	void BindRenderStates();
	void UnbindRenderStates();

private:
	int queue_;
	RenderState* states_[RenderStateCount];
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

protected:
	virtual void DrawCall(Mesh mesh);

private:
	unsigned particleCount_;
};
