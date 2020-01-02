#pragma once
#include "light.h"
#include "camera.h"
#include "screen.h"
#include "material.h"
#include "gameobject.h"

#include "cullingthread.h"
#include "internal/base/context.h"
#include "containers/dynamicarray.h"
#include "containers/concurrentqueue.h"

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

class Time;
class Profiler;
class Graphics;
class ShadowMap;
class RenderingThread;
class AmbientOcclusion;

class ShaderInternal;
class MaterialInternal;

class RenderingContext : public Context {
public:
	RenderingContext();
	~RenderingContext();

public:
	void ClearFrame();

	void SetTime(Time* value) { time_ = value; }
	Time* GetTime() { return time_; }

	void SetScene(Scene* value) { scene_ = value; }
	Scene* GetScene() { return scene_; }

	void SetProfiler(Profiler* value) { profiler_ = value; }
	Profiler* GetProfiler() { return profiler_ ; }

	void SetGraphics(Graphics* value) { graphics_ = value; }
	Graphics* GetGraphics() { return graphics_; }

	RenderingThread* GetRenderingThread() { return renderingThread_; }

	ShadowMap* GetShadowMap() { return shadowMap_; }
	AmbientOcclusion* GetAmbientOcclusion() { return ambientOcclusion_; }

	UniformState* GetUniformState() { return uniformState_; }
	FrameState* GetFrameState() { return frameState_; }

	Material* GetDepthMaterial() { return depthMaterial_.get(); }
	RenderTexture* GetOffscreenRenderTexture() { return offscreenRT_.get(); }

	CullingThread* GetCullingThread() { return cullingThreadQueue_.pop(); }
	void ReleaseCullingThread(CullingThread* thread) { cullingThreadQueue_.push(thread); }

protected:
	virtual bool Initialize();

private:
	void OnShaderCreated(ShaderInternal* shader);
	void OnMaterialShaderChanged(MaterialInternal* material);
	void OnScreenSizeChanged(uint width, uint height);

private:
	Time* time_ = nullptr;
	Scene* scene_ = nullptr;
	Profiler* profiler_ = nullptr;
	Graphics* graphics_ = nullptr;

	RenderingThread* renderingThread_;
	dynamic_array<CullingThread> cullingThreads_;
	concurrent_queue<CullingThread*> cullingThreadQueue_;

	ShadowMap* shadowMap_ = nullptr;
	AmbientOcclusion* ambientOcclusion_ = nullptr;

	FrameState* frameState_ = nullptr;
	UniformState* uniformState_ = nullptr;

	ref_ptr<Material> depthMaterial_;
	ref_ptr<RenderTexture> offscreenRT_;
};
