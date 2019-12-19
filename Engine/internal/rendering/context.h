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
	void Update();
	void CullingUpdate();

	void ClearFrame();

	ShadowMap* GetShadowMap() { return shadowMap_; }
	AmbientOcclusion* GetAmbientOcclusion() { return ambientOcclusion_; }

	UniformState* GetUniformState() { return uniformState_; }
	FrameState* GetFrameState() { return frameState_; }

	SharedUniformBuffers* GetSharedUniformBuffers() { return sharedUniformBuffers_; }

	Material* GetDepthMaterial() { return depthMaterial_.get(); }
	RenderTexture* GetOffscreenRenderTexture() { return offscreenRT_.get(); }

private:
	void Initialize();

	void OnShaderCreated(Shader* shader);
	void OnMaterialShaderChanged(Material* material);
	void OnScreenSizeChanged(uint width, uint height);

private:
	bool initialized_ = false;

	ShadowMap* shadowMap_ = nullptr;
	AmbientOcclusion* ambientOcclusion_ = nullptr;

	FrameState* frameState_ = nullptr;
	UniformState* uniformState_ = nullptr;

	// Uniform buffers used by all shaders.
	SharedUniformBuffers* sharedUniformBuffers_ = nullptr;

	ref_ptr<Material> depthMaterial_;
	ref_ptr<RenderTexture> offscreenRT_;
};
