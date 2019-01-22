#pragma once
#include "light.h"
#include "texture.h"
#include "material.h"
#include "renderer.h"
#include "gameobject.h"

#include "internal/rendering/pipeline.h"
#include "internal/rendering/matrixbuffer.h"
#include "internal/rendering/uniformbuffermanager.h"

class Sample;
class Pipeline;

enum RenderPassType {
	RenderPassTypeNone = -1,

	RenderPassTypeShadowDepth,

	RenderPassTypeForwardBackground,
	RenderPassTypeForwardDepth,
	RenderPassTypeForwardOpaque,
	RenderPassTypeForwardTransparent,

	RenderPassTypeDeferredGeometryPass,

	RenderPassTypeCount
};

struct RenderingMatrices {
	glm::vec3 cameraPos;
	glm::vec4 projParams;
	glm::mat4 projectionMatrix;
	glm::mat4 worldToCameraMatrix;
};

struct RenderingMaterials {
	Material ssao;
	Material ssaoTraversal;

	Material depth;
};

struct RenderingRenderTextures {
	RenderTexture aux1;
	RenderTexture aux2;
	RenderTexture target;
	MRTRenderTexture ssaoTraversal;
};

class ShadowParameters {
public:
	ShadowParameters();

public:
	void Update(Light light);

	Material GetMaterial();
	const glm::mat4& GetWorldToShadowMatrix();

private:
	glm::mat4 worldToShadowMatrix_;
	Material directionalLightShadowMaterial_;
};

struct RenderingParameters {
	RenderingParameters();
	~RenderingParameters();

	ShadowParameters* shadow;
	MatrixBuffer* matrixBuffer;

	IGameObject* camera;
	Rect normalizedRect;

	ClearType clearType;
	Color clearColor;

	RenderPath renderPath;
	DepthTextureMode depthTextureMode;

	RenderingMaterials materials;
	RenderingRenderTextures renderTextures;
};

struct RenderingPipelines {
	Light forwardBaseLight;

	Pipeline* depth;
	Pipeline* shadow;
	Pipeline* rendering;
	Pipeline* ssaoTraversal;
};

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

public:
	static RenderTexture GetSSAOTexture();
	static RenderTexture GetDepthTexture();
	static RenderTexture GetShadowDepthTexture();
	static TextureBuffer GetMatrixTextureBuffer();
	static UniformBufferManager* GetUniformBufferManager();

private:
	void OnPostRender();
	void OnImageEffects();

	void DepthPass(RenderingPipelines& pipelines);
	void SSAOPass(RenderingPipelines& pipelines);
	void SSAOTraversalPass(RenderingPipelines& pipelines);
	void ShadowPass(RenderingPipelines& pipelines);
	void RenderPass(RenderingPipelines& pipelines);
	void UpdateUniformBuffers(const RenderingMatrices& matrices, RenderingPipelines& pipelines);

	void UpdateForwardBaseLightUniformBuffer(Light light);
	void UpdateTransformsUniformBuffer(const RenderingMatrices& matrices);

	void CreateAuxMaterial(Material& material, const std::string& shaderPath, uint renderQueue);

private:
	RenderingParameters* p_;
	std::vector<GameObject> gameObjects_;

	Sample* ssaoSample;
	Sample* ssaoTraversalSample;

	Sample* depthSample;
	Sample* shadowSample;
	Sample* renderingSample;
};

// SUEDE TODO: multi-thread rendering.
class RenderableTraits/* : public ZThread::Runnable, public DirtyBits*/ {
public:
	RenderableTraits(RenderingParameters* p/*RenderingListener* listener*/);
	~RenderableTraits();

public:
	RenderingPipelines& GetPipelines() { return pipelines_; }
	void Traits(const std::vector<GameObject>& gameObjects, const RenderingMatrices& matrices);
	void Clear();

private:
	void InitializeSSAOKernel();

	void ForwardRendering(Pipeline* pl, const std::vector<GameObject>& gameObjects, Light forwardBase, const std::vector<Light>& forwardAdd);
	void DeferredRendering(Pipeline* pl, const std::vector<GameObject>& gameObjects, Light forwardBase, const std::vector<Light>& forwardAdd);

	void InitializeDeferredRender();
	void RenderDeferredGeometryPass(Pipeline* pl, const std::vector<GameObject>& gameObjects);

	void RenderSkybox(Pipeline* pl);

	RenderTexture GetActiveRenderTarget();

	void SSAOPass(Pipeline* pl);

	void ForwardPass(Pipeline* pl, const std::vector<GameObject>& gameObjects);
	void ForwardDepthPass(Pipeline* pl);

	void RenderGameObject(Pipeline* pl, GameObject go, Renderer renderer);
	void RenderSubMesh(Pipeline* pl, GameObject go, int subMeshIndex, Material material, int pass);

	void RenderForwardAdd(Pipeline* pl, const std::vector<GameObject>& gameObjects, const std::vector<Light>& lights);
	void RenderForwardBase(Pipeline* pl, const std::vector<GameObject>& gameObjects, Light light);

	void RenderDecals(Pipeline* pl);

	void ReplaceMaterials(Pipeline* pl, Material material);

	void GetLights(Light& forwardBase, std::vector<Light>& forwardAdd);

private:
	RenderingParameters* p_;
	RenderingMatrices matrices_;

	/*RenderingListener* listener_;*/

	RenderingPipelines pipelines_;
	Sample *push_renderables, *forward_pass, *get_renderable_game_objects;
};
