#include "screen.h"
#include "builtinproperties.h"
#include "sharedtexturemanager.h"
#include "internal/base/renderdefines.h"

SharedTextureManager::SharedTextureManager() {
	uint w = Screen::GetWidth();
	uint h = Screen::GetHeight();

	depth_ = NewRenderTexture();
	depth_->Create(RenderTextureFormat::Depth, w, h);

	shadowDepth_ = NewRenderTexture();
	shadowDepth_->Create(RenderTextureFormat::Shadow, w, h);

	ssao_ = NewRenderTexture();
	ssao_->Create(RenderTextureFormat::RgbF, w, h);

	matrixBuffer_ = NewTextureBuffer();
	matrixBuffer_->Create(INIT_RENDERABLE_CAPACITY * sizeof(glm::mat4) * 2);
}

void SharedTextureManager::Attach(Material material) {
	material->SetTexture(BuiltinProperties::SSAOTexture, ssao_);
	material->SetTexture(BuiltinProperties::DepthTexture, depth_);
	material->SetTexture(BuiltinProperties::ShadowDepthTexture, shadowDepth_);
	material->SetTexture(BuiltinProperties::MatrixTextureBuffer, matrixBuffer_);
}
