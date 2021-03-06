#include "renderingcontext.h"

#include "resources.h"
#include "renderingthread.h"
#include "builtinproperties.h"

#include "shadowmap.h"
#include "ambientocclusion.h"
#include "shareduniformbuffers.h"

#include "internal/base/renderdefines.h"
#include "internal/base/shaderinternal.h"
#include "internal/base/materialinternal.h"

UniformState::UniformState(Context* context) {
	uint w = Screen::GetWidth(), h = Screen::GetHeight();

	depthTexture = new RenderTexture();
	depthTexture->Create(RenderTextureFormat::Depth, w, h);

	depthNormalsTexture = new RenderTexture();
	depthNormalsTexture->Create(RenderTextureFormat::Rgba, w, h);

	shadowDepthTexture = new RenderTexture();
	shadowDepthTexture->Create(RenderTextureFormat::Shadow, w, h);

	ambientOcclusionTexture = new RenderTexture();
	ambientOcclusionTexture->Create(RenderTextureFormat::RgbF, w, h);

	matrixTextureBuffer = new TextureBuffer();
	matrixTextureBuffer->Create(INIT_RENDERABLE_CAPACITY * sizeof(Matrix4) * 2);

	uniformBuffers = new SharedUniformBuffers(context);
}

UniformState::~UniformState() {
	delete uniformBuffers;
}

RenderingContext::RenderingContext() { 
}

RenderingContext::~RenderingContext() {
	OnDestroy();

	delete frameState_;
	delete uniformState_;
	delete shadowMap_;
	delete ambientOcclusion_;

	Screen::sizeChanged.unsubscribe(this);
	ShaderInternal::shaderCreated.unsubscribe(this);
	MaterialInternal::shaderChanged.unsubscribe(this);
}

void RenderingContext::OnDestroy() {
	if (renderingThread_ != nullptr) {
		renderingThread_->Stop();
		delete renderingThread_;
		renderingThread_ = nullptr;
	}

	delete cullingThread_;
	cullingThread_ = nullptr;
}

bool RenderingContext::Initialize() {
	if (!Context::Initialize()) { return false; }

	cullingThread_ = new CullingThread(this);
	renderingThread_ = new RenderingThread(this);

	frameState_ = new FrameState();
	uniformState_ = new UniformState(this);

	shadowMap_ = new ShadowMap(uniformState_->shadowDepthTexture.get());
	ambientOcclusion_ = new AmbientOcclusion(graphics_, uniformState_->ambientOcclusionTexture.get());

	offscreenRT_ = new RenderTexture();
	offscreenRT_->Create(RenderTextureFormat::Rgba, Screen::GetWidth(), Screen::GetHeight());

	depthMaterial_ = new Material();
	depthMaterial_->SetShader(Shader::Find("builtin/depth"));
	depthMaterial_->SetRenderQueue((int)RenderQueue::Background - 300);

	depthNormalsMaterial_ = new Material();
	depthNormalsMaterial_->SetShader(Shader::Find("builtin/depth_normals"));
	depthNormalsMaterial_->SetRenderQueue((int)RenderQueue::Background - 300);

	ShaderInternal::shaderCreated.subscribe(this, &RenderingContext::OnShaderCreated);
	MaterialInternal::shaderChanged.subscribe(this, &RenderingContext::OnMaterialShaderChanged);

	Screen::sizeChanged.subscribe(this, &RenderingContext::OnScreenSizeChanged);
	return true;
}

void RenderingContext::OnShaderCreated(ShaderInternal* shader) {
	uniformState_->uniformBuffers->Attach(shader);
}

void RenderingContext::OnMaterialShaderChanged(MaterialInternal* material) {
	material->SetTexture(BuiltinProperties::SSAOTexture, uniformState_->ambientOcclusionTexture.get());
	material->SetTexture(BuiltinProperties::ShadowDepthTexture, uniformState_->shadowDepthTexture.get());
	material->SetTexture(BuiltinProperties::CameraDepthTexture, uniformState_->depthTexture.get());
	material->SetTexture(BuiltinProperties::CameraDepthNormalsTexture, uniformState_->depthNormalsTexture.get());
	material->SetTexture(BuiltinProperties::MatrixTextureBuffer, uniformState_->matrixTextureBuffer.get());
}

void RenderingContext::OnScreenSizeChanged(uint width, uint height) {
	offscreenRT_->Resize(width, height);
	ambientOcclusion_->Resize(width, height);
	uniformState_->depthTexture->Resize(width, height);
	uniformState_->depthNormalsTexture->Resize(width, height);
}

// SUEDE TODO Remove me.
#include "graphics.h"
#include "internal/base/textureinternal.h"

void RenderingContext::ClearFrame() {
	offscreenRT_->Clear(frameState_->normalizedRect, frameState_->clearColor, 1, -1);

	uniformState_->depthTexture->Clear(Rect::unit, Color::black, 1, -1);
	//_suede_drptr(uniformState_->depthTexture.get())->__tmpApplyClear();

	uniformState_->depthNormalsTexture->Clear(Rect::unit, Color::black, 1, -1);

	RenderTexture* target = frameState_->targetTexture.get();
	if (target == nullptr) { target = RenderTexture::GetDefault(); }
	target->Clear(frameState_->normalizedRect, frameState_->clearColor, 1, 0);
}
