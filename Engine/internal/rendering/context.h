#pragma once
#include "light.h"
#include "camera.h"
#include "screen.h"
#include "material.h"
#include "gameobject.h"
#include "internal/rendering/shareduniformbuffers.h"

struct UniformState {
	UniformState();

	// Uniforms used by all materials.
	ref_ptr<RenderTexture> depthTexture;
	ref_ptr<RenderTexture> shadowDepthTexture;
	ref_ptr<TextureBuffer> matrixTextureBuffer;
	ref_ptr<RenderTexture> ambientOcclusionTexture;
};

struct FrameState {
	GameObject* camera = nullptr;
	Light* forwardBaseLight = nullptr;

	Rect normalizedRect = Rect(0, 0, 1, 1);

	ClearType clearType = ClearType::Color;
	Color clearColor = Color::black;

	RenderPath renderPath = RenderPath::Forward;

	DepthTextureMode depthTextureMode = DepthTextureMode::None;
	ref_ptr<RenderTexture> targetTexture;
};

class ShadowMap;
class AmbientOcclusion;

class Context {
public:
	Context();
	~Context();

public:
	static Context* GetCurrent();
	static void SetCurrent(Context* value);

public:
	void ClearFrame();

	ShadowMap* GetShadowMap() { return shadowMap; }
	AmbientOcclusion* GetAmbientOcclusion() { return ambientOcclusion; }

	UniformState* GetUniformState() { return uniformState; }
	FrameState* GetFrameState() { return frameState; }

	SharedUniformBuffers* GetSharedUniformBuffers() { return sharedUniformBuffers; }

	Material* GetDepthMaterial() { return depthMaterial.get(); }
	RenderTexture* GetOffscreenRenderTexture() { return offscreenRT.get(); }

private:
	void Initialize();

	void OnShaderCreated(Shader* shader);
	void OnMaterialShaderChanged(Material* material);
	void OnScreenSizeChanged(uint width, uint height);

private:
	bool initialized = false;

	ShadowMap* shadowMap = nullptr;
	AmbientOcclusion* ambientOcclusion = nullptr;

	FrameState* frameState = nullptr;
	UniformState* uniformState = nullptr;

	// Uniform buffers used by all shaders.
	SharedUniformBuffers* sharedUniformBuffers = nullptr;

	ref_ptr<Material> depthMaterial;
	ref_ptr<RenderTexture> offscreenRT;
};
