#include "context.h"

#include "resources.h"
#include "builtinproperties.h"

#include "internal/base/gl.h"
#include "internal/base/renderdefines.h"
#include "internal/base/shaderinternal.h"
#include "internal/base/materialinternal.h"

#include "internal/rendering/shadowmap.h"
#include "internal/rendering/ambientocclusion.h"

UniformState::UniformState() {
	uint w = Screen::GetWidth(), h = Screen::GetHeight();

	depthTexture = new RenderTexture();
	depthTexture->Create(RenderTextureFormat::Depth, w, h);

	shadowDepthTexture = new RenderTexture();
	shadowDepthTexture->Create(RenderTextureFormat::Shadow, w, h);

	ambientOcclusionTexture = new RenderTexture();
	ambientOcclusionTexture->Create(RenderTextureFormat::RgbF, w, h);

	matrixTextureBuffer = new TextureBuffer();
	matrixTextureBuffer->Create(INIT_RENDERABLE_CAPACITY * sizeof(Matrix4) * 2);

}

Context::Context() { }

Context::~Context() {
	delete frameState_;
	delete uniformState_;
	delete shadowMap_;
	delete ambientOcclusion_;
	delete sharedUniformBuffers_;

	Screen::sizeChanged.unsubscribe(this);
	ShaderInternal::shaderCreated.unsubscribe(this);
	MaterialInternal::shaderChanged.unsubscribe(this);
}

void Context::Initialize() {
	if (initialized_) { return; }

	frameState_ = new FrameState();
	uniformState_ = new UniformState();
	sharedUniformBuffers_ = new SharedUniformBuffers();

	shadowMap_ = new ShadowMap(uniformState_->shadowDepthTexture.get());
	ambientOcclusion_ = new AmbientOcclusion(uniformState_->ambientOcclusionTexture.get());

	offscreenRT_ = new RenderTexture();
	offscreenRT_->Create(RenderTextureFormat::Rgba, Screen::GetWidth(), Screen::GetHeight());

	depthMaterial_ = new Material();
	depthMaterial_->SetShader(Shader::Find("builtin/depth"));
	depthMaterial_->SetRenderQueue((int)RenderQueue::Background - 300);

	ShaderInternal::shaderCreated.subscribe(this, &Context::OnShaderCreated);
	MaterialInternal::shaderChanged.subscribe(this, &Context::OnMaterialShaderChanged);

	Screen::sizeChanged.subscribe(this, &Context::OnScreenSizeChanged);
	initialized_ = true;
}

void Context::OnShaderCreated(Shader* shader) {
	sharedUniformBuffers_->Attach(shader);
}

void Context::OnMaterialShaderChanged(Material* material) {
	material->SetTexture(BuiltinProperties::SSAOTexture, uniformState_->ambientOcclusionTexture.get());
	material->SetTexture(BuiltinProperties::ShadowDepthTexture, uniformState_->shadowDepthTexture.get());
	material->SetTexture(BuiltinProperties::DepthTexture, uniformState_->depthTexture.get());
	material->SetTexture(BuiltinProperties::MatrixTextureBuffer, uniformState_->matrixTextureBuffer.get());
}

void Context::OnScreenSizeChanged(uint width, uint height) {
	offscreenRT_->Resize(width, height);
	ambientOcclusion_->Resize(width, height);
	uniformState_->depthTexture->Resize(width, height);
}

void Context::ClearFrame() {
	offscreenRT_->Clear(frameState_->normalizedRect, frameState_->clearColor, 1);

	uniformState_->depthTexture->Clear(Rect(0, 0, 1, 1), Color::black, 1);

	RenderTexture* target = frameState_->targetTexture.get();
	if (!target) { target = RenderTexture::GetDefault(); }
	target->Clear(frameState_->normalizedRect, frameState_->clearColor, 1);
}

static Context* current_;
Context* Context::GetCurrent() { return current_; }
void Context::SetCurrent(Context* value) { (current_ = value)->Initialize(); }
