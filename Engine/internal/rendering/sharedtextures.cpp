#include "screen.h"
#include "variables.h"
#include "sharedtextures.h"
#include "internal/base/renderdefines.h"

SharedTextures::SharedTextures() {
	uint w = Screen::instance()->GetWidth();
	uint h = Screen::instance()->GetHeight();

	depth_ = NewRenderTexture();
	depth_->Create(RenderTextureFormatDepth, w, h);

	shadowDepth_ = NewRenderTexture();
	shadowDepth_->Create(RenderTextureFormatShadow, w, h);

	matrixBuffer_ = NewTextureBuffer();
	matrixBuffer_->Create(INIT_RENDERABLE_CAPACITY * sizeof(glm::mat4) * 2);

	ssao_ = NewRenderTexture();
	ssao_->Create(RenderTextureFormatRgbHDR, w, h);
}

void SharedTextures::Attach(Material material) {
	material->SetTexture(Variables::SSAOTexture, ssao_);
	material->SetTexture(Variables::DepthTexture, depth_);
	material->SetTexture(Variables::ShadowDepthTexture, shadowDepth_);
	material->SetTexture(Variables::MatrixTextureBuffer, matrixBuffer_);
}
