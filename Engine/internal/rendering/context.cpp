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
	delete frameState;
	delete uniformState;
	delete shadowMap;
	delete ambientOcclusion;
	delete sharedUniformBuffers;

	Screen::sizeChanged.unsubscribe(this);
	ShaderInternal::shaderCreated.unsubscribe(this);
	MaterialInternal::shaderChanged.unsubscribe(this);
}

void Context::Initialize() {
	if (initialized) { return; }

	frameState = new FrameState();
	uniformState = new UniformState();
	sharedUniformBuffers = new SharedUniformBuffers();

	shadowMap = new ShadowMap(uniformState->shadowDepthTexture.get());
	ambientOcclusion = new AmbientOcclusion(uniformState->ambientOcclusionTexture.get());

	offscreenRT = new RenderTexture();
	offscreenRT->Create(RenderTextureFormat::Rgba, Screen::GetWidth(), Screen::GetHeight());

	depthMaterial = new Material();
	depthMaterial->SetShader(Shader::Find("builtin/depth"));
	depthMaterial->SetRenderQueue((int)RenderQueue::Background - 300);

	ShaderInternal::shaderCreated.subscribe(this, &Context::OnShaderCreated);
	MaterialInternal::shaderChanged.subscribe(this, &Context::OnMaterialShaderChanged);

	Screen::sizeChanged.subscribe(this, &Context::OnScreenSizeChanged);
	initialized = true;
}

void Context::OnShaderCreated(Shader* shader) {
	sharedUniformBuffers->Attach(shader);
}

void Context::OnMaterialShaderChanged(Material* material) {
	material->SetTexture(BuiltinProperties::SSAOTexture, uniformState->ambientOcclusionTexture.get());
	material->SetTexture(BuiltinProperties::ShadowDepthTexture, uniformState->shadowDepthTexture.get());
	material->SetTexture(BuiltinProperties::DepthTexture, uniformState->depthTexture.get());
	material->SetTexture(BuiltinProperties::MatrixTextureBuffer, uniformState->matrixTextureBuffer.get());
}

void Context::OnScreenSizeChanged(uint width, uint height) {
	offscreenRT->Resize(width, height);
	ambientOcclusion->Resize(width, height);
	uniformState->depthTexture->Resize(width, height);
}

void Context::ClearFrame() {
	offscreenRT->Clear(frameState->normalizedRect, frameState->clearColor, 1);

	uniformState->depthTexture->Clear(Rect(0, 0, 1, 1), Color::black, 1);

	RenderTexture* target = frameState->targetTexture.get();
	if (!target) { target = RenderTexture::GetDefault(); }
	target->Clear(frameState->normalizedRect, frameState->clearColor, 1);
}

static Context* current_;
Context* Context::GetCurrent() { return current_; }
void Context::SetCurrent(Context* value) { (current_ = value)->Initialize(); }
