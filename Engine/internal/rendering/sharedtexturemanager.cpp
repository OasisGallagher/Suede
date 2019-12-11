#include "screen.h"
#include "builtinproperties.h"
#include "sharedtexturemanager.h"
#include "internal/base/renderdefines.h"

SharedTextureManager::SharedTextureManager() {
	uint w = Screen::GetWidth();
	uint h = Screen::GetHeight();

	depth_ = new RenderTexture();
	depth_->Create(RenderTextureFormat::Depth, w, h);

	shadowDepth_ = new RenderTexture();
	shadowDepth_->Create(RenderTextureFormat::Shadow, w, h);

	ssao_ = new RenderTexture();
	ssao_->Create(RenderTextureFormat::RgbF, w, h);

	matrixBuffer_ = new TextureBuffer();
	matrixBuffer_->Create(INIT_RENDERABLE_CAPACITY * sizeof(Matrix4) * 2);
}

SharedTextureManager::~SharedTextureManager() {
}

void SharedTextureManager::Attach(Material* material) {
	material->SetTexture(BuiltinProperties::SSAOTexture, ssao_.get());
	material->SetTexture(BuiltinProperties::DepthTexture, depth_.get());
	material->SetTexture(BuiltinProperties::ShadowDepthTexture, shadowDepth_.get());
	material->SetTexture(BuiltinProperties::MatrixTextureBuffer, matrixBuffer_.get());
}
