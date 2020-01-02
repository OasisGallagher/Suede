#pragma once
#include <set>

#include "light.h"
#include "texture.h"
#include "material.h"
#include "renderer.h"
#include "gameobject.h"
#include "internal/rendering/pipeline.h"

class Sample;
class Pipeline;
class Graphics;
class ShadowMap;
class AmbientOcclusion;
class RenderingContext;
class SharedUniformBuffers;

struct RenderingMatrices {
	Vector3 cameraPos;
	Vector4 projParams;
	Matrix4 projectionMatrix;
	Matrix4 worldToCameraMatrix;
};

struct RenderingPipelines {
	RenderingPipelines(RenderingContext* context);
	~RenderingPipelines();

	void Clear();

	Pipeline* depth;
	Pipeline* shadow;
	Pipeline* rendering;
	Pipeline* ssaoTraversal;
};

class ImageEffect;
class RenderingThread;

class RenderingThread {
public:
	RenderingThread(RenderingContext* context);
	~RenderingThread();

public:
	void Render(RenderingPipelines* pipelines, const RenderingMatrices& matrices);
	void Stop();

	std::thread::id GetThreadID() { return threadID_; }

private:
	void ThreadProc();
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
	bool stopped_ = false;

	std::mutex mutex_;
	std::thread thread_;
	std::condition_variable cond_;

	Profiler* profiler_;
	Graphics* graphics_;
	std::thread::id threadID_;
	RenderingContext* context_;
};

class PipelineBuilder {
public:
	PipelineBuilder(RenderingContext* context);
	~PipelineBuilder() {}

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
	Profiler* profiler_;
	Graphics* graphics_;
	RenderingContext* context_;
	RenderingMatrices matrices_;
};
