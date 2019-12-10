#include "screen.h"
#include "builtinproperties.h"
#include "sharedtexturemanager.h"
#include "internal/base/renderdefines.h"

SharedTextureManager::SharedTextureManager() {
	uint w = Screen::GetWidth();
	uint h = Screen::GetHeight();

	depth_ = new IRenderTexture();
	depth_->Create(RenderTextureFormat::Depth, w, h);

	shadowDepth_ = new IRenderTexture();
	shadowDepth_->Create(RenderTextureFormat::Shadow, w, h);

	ssao_ = new IRenderTexture();
	ssao_->Create(RenderTextureFormat::RgbF, w, h);

	matrixBuffer_ = new ITextureBuffer();
	matrixBuffer_->Create(INIT_RENDERABLE_CAPACITY * sizeof(Matrix4) * 2);
}

SharedTextureManager::~SharedTextureManager() {
}

void SharedTextureManager::Attach(Material material) {
	material->SetTexture(BuiltinProperties::SSAOTexture, ssao_);
	material->SetTexture(BuiltinProperties::DepthTexture, depth_);
	material->SetTexture(BuiltinProperties::ShadowDepthTexture, shadowDepth_);
	material->SetTexture(BuiltinProperties::MatrixTextureBuffer, matrixBuffer_);
}
