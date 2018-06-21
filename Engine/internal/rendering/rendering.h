#pragma once
#include "light.h"
#include "entity.h"
#include "texture.h"
#include "material.h"
//#include "tools/dirtybits.h"
#include "internal/rendering/pipeline.h"

class Sample;
class Pipeline;
class ImageEffect;

enum RenderPass {
	RenderPassNone = -1,

	RenderPassShadowDepth,

	RenderPassForwardBackground,
	RenderPassForwardDepth,
	RenderPassForwardOpaque,
	RenderPassForwardTransparent,

	RenderPassDeferredGeometryPass,

	RenderPassCount
};

struct RenderingMatrices {
	glm::vec3 position;
	glm::mat4 projectionMatrix;
	glm::mat4 worldToCameraMatrix;
};

struct RenderingMaterials {
	// TODO: Common material.
	Material decal;
	Material depth;
};

struct RenderingRenderTextures {
	RenderTexture aux1;
	RenderTexture aux2;
	RenderTexture depth;
	RenderTexture target;
};

struct RenderingParameters {
	RenderingParameters();

	Rect normalizedRect;

	ClearType clearType;
	glm::vec3 clearColor;

	RenderPath renderPath;
	DepthTextureMode depthTextureMode;

	RenderingMaterials materials;
	RenderingRenderTextures renderTextures;

	std::vector<ImageEffect*> imageEffects;
};

struct RenderingPipelines {
	Light forwardBaseLight;

	Pipeline* depth;
	Pipeline* shadow;
	Pipeline* rendering;
};

class Rendering;
class RenderingListener {
public:
	virtual void OnRenderingFinished() = 0;
};

class Rendering {
public:
	Rendering(RenderingParameters* p);

public:
	void Render(RenderingPipelines& pipelines, const RenderingMatrices& matrices);

	void Resize(uint width, uint height);
	void ClearRenderTextures();

private:
	void OnPostRender();
	void OnImageEffects();

	void UpdateForwardBaseLightUniformBuffer(Light light);
	void UpdateTransformsUniformBuffer(const RenderingMatrices& matrices);

	void CreateAuxMaterial(Material& material, const std::string& shaderPath, uint renderQueue);

private:
	RenderingParameters* p_;
	std::vector<Entity> entities_;
};

// TODO: multi-thread rendering.
class RenderableTraits/* : public ZThread::Runnable, public DirtyBits*/ {
public:
	RenderableTraits(RenderingParameters* p/*RenderingListener* listener*/);
	~RenderableTraits();

public:
	RenderingPipelines& GetPipelines() { return pipelines_; }
	void Traits(std::vector<Entity>& entities, const RenderingMatrices& matrices);

private:
	void ForwardRendering(Pipeline* pl, RenderTexture target, const std::vector<Entity>& entities, Light forwardBase, const std::vector<Light>& forwardAdd);
	void DeferredRendering(Pipeline* pl, RenderTexture target, const std::vector<Entity>& entities, Light forwardBase, const std::vector<Light>& forwardAdd);

	void InitializeDeferredRender();
	void RenderDeferredGeometryPass(Pipeline* pl, RenderTexture target, const std::vector<Entity>& entities);

	void RenderSkybox(Pipeline* pl, RenderTexture target);

	RenderTexture GetActiveRenderTarget();

	void ForwardPass(Pipeline* pl, RenderTexture target, const std::vector<Entity>& entities);
	void ForwardDepthPass(Pipeline* pl);

	void RenderEntity(Pipeline* pl, RenderTexture target, Entity entity, Renderer renderer);
	void RenderSubMesh(Pipeline* pl, RenderTexture target, Entity entity, int subMeshIndex, Material material, int pass);

	void RenderForwardAdd(Pipeline* pl, const std::vector<Entity>& entities, const std::vector<Light>& lights);
	void RenderForwardBase(Pipeline* pl, RenderTexture target, const std::vector<Entity>& entities, Light light);

	void RenderDecals(Pipeline* pl, RenderTexture target);

	void GetLights(Light& forwardBase, std::vector<Light>& forwardAdd);

private:
	RenderingParameters* p_;
	RenderingMatrices matrices_;

	/*RenderingListener* listener_;*/

	RenderingPipelines pipelines_;
	Sample *push_renderables, *forward_pass, *get_renderable_entities;
};
