#include "renderingcontext.h"

#include "resources.h"
#include "builtinproperties.h"

#include "internal/base/renderdefines.h"
#include "internal/base/shaderinternal.h"
#include "internal/base/materialinternal.h"

#include "internal/rendering/shadowmap.h"
#include "internal/rendering/ambientocclusion.h"
#include "internal/rendering/shareduniformbuffers.h"

UniformState::UniformState(Context* context) {
	uint w = Screen::GetWidth(), h = Screen::GetHeight();

	depthTexture = new RenderTexture();
	depthTexture->Create(RenderTextureFormat::Depth, w, h);

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

RenderingContext::RenderingContext() { }

RenderingContext::~RenderingContext() {
	delete frameState_;
	delete uniformState_;
	delete shadowMap_;
	delete ambientOcclusion_;

	Screen::sizeChanged.unsubscribe(this);
	ShaderInternal::shaderCreated.unsubscribe(this);
	MaterialInternal::shaderChanged.unsubscribe(this);
}

bool RenderingContext::Initialize() {
	if (!Context::Initialize()) { return false; }

	frameState_ = new FrameState();
	uniformState_ = new UniformState(this);

	shadowMap_ = new ShadowMap(uniformState_->shadowDepthTexture.get());
	ambientOcclusion_ = new AmbientOcclusion(uniformState_->ambientOcclusionTexture.get());

	offscreenRT_ = new RenderTexture();
	offscreenRT_->Create(RenderTextureFormat::Rgba, Screen::GetWidth(), Screen::GetHeight());

	depthMaterial_ = new Material();
	depthMaterial_->SetShader(Shader::Find("builtin/depth"));
	depthMaterial_->SetRenderQueue((int)RenderQueue::Background - 300);

	ShaderInternal::shaderCreated.subscribe(this, &RenderingContext::OnShaderCreated);
	MaterialInternal::shaderChanged.subscribe(this, &RenderingContext::OnMaterialShaderChanged);

	Screen::sizeChanged.subscribe(this, &RenderingContext::OnScreenSizeChanged);
	return true;
}

void RenderingContext::OnShaderCreated(Shader* shader) {
	uniformState_->uniformBuffers->Attach(shader);
}

void RenderingContext::OnMaterialShaderChanged(Material* material) {
	material->SetTexture(BuiltinProperties::SSAOTexture, uniformState_->ambientOcclusionTexture.get());
	material->SetTexture(BuiltinProperties::ShadowDepthTexture, uniformState_->shadowDepthTexture.get());
	material->SetTexture(BuiltinProperties::DepthTexture, uniformState_->depthTexture.get());
	material->SetTexture(BuiltinProperties::MatrixTextureBuffer, uniformState_->matrixTextureBuffer.get());
}

void RenderingContext::OnScreenSizeChanged(uint width, uint height) {
	offscreenRT_->Resize(width, height);
	ambientOcclusion_->Resize(width, height);
	uniformState_->depthTexture->Resize(width, height);
}

void RenderingContext::ClearFrame() {
	offscreenRT_->Clear(frameState_->normalizedRect, frameState_->clearColor, 1);

	uniformState_->depthTexture->Clear(Rect(0, 0, 1, 1), Color::black, 1);

	RenderTexture* target = frameState_->targetTexture.get();
	if (!target) { target = RenderTexture::GetDefault(); }
	target->Clear(frameState_->normalizedRect, frameState_->clearColor, 1);
}

void RenderingContext::Update() {

}

void RenderingContext::CullingUpdate() {

}
