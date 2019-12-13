#pragma once
#include <set>

#include "light.h"
#include "texture.h"
#include "material.h"
#include "renderer.h"
#include "gameobject.h"
#include "internal/rendering/pipeline.h"

class Sample;
class Context;
class Pipeline;
class ShadowMap;
class AmbientOcclusion;
class SharedUniformBuffers;

struct RenderingMatrices {
	Vector3 cameraPos;
	Vector4 projParams;
	Matrix4 projectionMatrix;
	Matrix4 worldToCameraMatrix;
};

struct RenderingPipelines {
	RenderingPipelines(Context* context);
	~RenderingPipelines();

	void Clear();

	Pipeline* depth;
	Pipeline* shadow;
	Pipeline* rendering;
	Pipeline* ssaoTraversal;
};

class Rendering;
class ImageEffect;

class Rendering {
public:
	Rendering(Context* context);

public:
	void Render(RenderingPipelines* pipelines, const RenderingMatrices& matrices);

private:
	void OnPostRender();
	void OnImageEffects(const std::vector<ImageEffect*>& effects);

	void DepthPass(RenderingPipelines* pipelines);
	void SSAOPass(RenderingPipelines* pipelines);
	void SSAOTraversalPass(RenderingPipelines* pipelines);
	void ShadowPass(RenderingPipelines* pipelines);
	void RenderPass(RenderingPipelines* pipelines);
	void UpdateUniformBuffers(RenderingPipelines* pipelines, const RenderingMatrices& matrices);

	void UpdateForwardBaseLightUniformBuffer(Light* light);
	void UpdateTransformsUniformBuffer(const RenderingMatrices& matrices);

private:
	Context* context_;

	Sample* ssaoSample;
	Sample* ssaoTraversalSample;

	Sample* depthSample;
	Sample* shadowSample;
	Sample* renderingSample;
};

class PipelineBuilder {
public:
	PipelineBuilder(Context* context);
	~PipelineBuilder();

public:
	void Build(RenderingPipelines* pipelines, std::vector<GameObject*>& gameObjects, const RenderingMatrices& matrices);

private:
	void ForwardRendering(Pipeline* pl, const std::vector<GameObject*>& gameObjects, Light* forwardBase, const std::vector<Light*>& forwardAdd);
	void DeferredRendering(Pipeline* pl, const std::vector<GameObject*>& gameObjects, Light* forwardBase, const std::vector<Light*>& forwardAdd);

	void InitializeDeferredRender();
	void RenderDeferredGeometryPass(Pipeline* pl, const std::vector<GameObject*>& gameObjects);

	void RenderSkybox(Pipeline* pl);

	RenderTexture* GetActiveRenderTarget();

	void SSAOPass(Pipeline* pl);

	void ForwardPass(Pipeline* pl, const std::vector<GameObject*>& gameObjects);
	void ForwardDepthPass(Pipeline* pl);

	void RenderGameObject(Pipeline* pl, GameObject* go, Renderer* renderer);
	void RenderSubMesh(Pipeline* pl, GameObject* go, int subMeshIndex, Material* material, int pass);

	void RenderForwardAdd(Pipeline* pl, const std::vector<GameObject*>& gameObjects, const std::vector<Light*>& lights);
	void RenderForwardBase(Pipeline* pl, const std::vector<GameObject*>& gameObjects, Light* light);

	void RenderDecals(Pipeline* pl);

	void ReplaceMaterials(Pipeline* pl, Material* material);

	void GetLights(Light*& forwardBase, std::vector<Light*>& forwardAdd);

private:
	Context* context_;
	RenderingMatrices matrices_;

	Sample *push_renderables, *forward_pass, *get_renderable_game_objects;
};
