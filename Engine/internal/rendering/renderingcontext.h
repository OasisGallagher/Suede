#pragma once
#include "light.h"
#include "camera.h"
#include "screen.h"
#include "material.h"
#include "gameobject.h"

#include "internal/base/context.h"

class SharedUniformBuffers;

/**
 * Uniforms used by all materials.
 */
struct UniformState {
	UniformState(Context* context);
	~UniformState();

	ref_ptr<RenderTexture> depthTexture;
	ref_ptr<RenderTexture> shadowDepthTexture;
	ref_ptr<TextureBuffer> matrixTextureBuffer;
	ref_ptr<RenderTexture> ambientOcclusionTexture;

	// Uniform buffers used by all shaders.
	SharedUniformBuffers* uniformBuffers;
};

/**
 * State information about the current frame.
 */
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

class RenderingContext : public Context {
public:
	RenderingContext();
	~RenderingContext();

public:
	virtual void Update();
	virtual void CullingUpdate();

public:
	void ClearFrame();

	ShadowMap* GetShadowMap() { return shadowMap_; }
	AmbientOcclusion* GetAmbientOcclusion() { return ambientOcclusion_; }

	UniformState* GetUniformState() { return uniformState_; }
	FrameState* GetFrameState() { return frameState_; }

	Material* GetDepthMaterial() { return depthMaterial_.get(); }
	RenderTexture* GetOffscreenRenderTexture() { return offscreenRT_.get(); }

protected:
	virtual bool Initialize();

private:
	void OnShaderCreated(Shader* shader);
	void OnMaterialShaderChanged(Material* material);
	void OnScreenSizeChanged(uint width, uint height);

private:
	ShadowMap* shadowMap_ = nullptr;
	AmbientOcclusion* ambientOcclusion_ = nullptr;

	FrameState* frameState_ = nullptr;
	UniformState* uniformState_ = nullptr;

	ref_ptr<Material> depthMaterial_;
	ref_ptr<RenderTexture> offscreenRT_;
};
