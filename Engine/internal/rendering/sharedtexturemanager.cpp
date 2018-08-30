#include "screen.h"
#include "variables.h"
#include "sharedtexturemanager.h"
#include "internal/base/renderdefines.h"

SharedTextureManager::SharedTextureManager() {
	uint w = Screen::instance()->GetWidth();
	uint h = Screen::instance()->GetHeight();

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
	material->SetTexture(Variables::SSAOTexture, ssao_);
	material->SetTexture(Variables::DepthTexture, depth_);
	material->SetTexture(Variables::ShadowDepthTexture, shadowDepth_);
	material->SetTexture(Variables::MatrixTextureBuffer, matrixBuffer_);
}
