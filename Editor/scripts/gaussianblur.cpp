#include "resources.h"
#include "gaussianblur.h"

SUEDE_DEFINE_COMPONENT(GaussianBlur, ImageEffect)

void GaussianBlur::Awake() {
	amount_.reset(5, 1, 10);
	material_ = new IMaterial();
	material_->SetShader(Resources::FindShader("gaussianblur"));
}

void GaussianBlur::OnRenderImage(RenderTexture src, RenderTexture dest, const Rect& normalizedRect) {
	int horizontal = 1;
	int width = src->GetWidth(), height = src->GetHeight();

	// temporary render texture.
	RenderTexture buffers[2];
	buffers[0] = RenderTextureUtility::GetTemporary(RenderTextureFormat::RgbF, width, height);
	buffers[1] = RenderTextureUtility::GetTemporary(RenderTextureFormat::RgbF, width, height);

	for (int i = 0; i < amount_.get_value(); ++i) {
		material_->SetBool("horizontal", horizontal != 0);
		Graphics::Blit(src, buffers[horizontal], material_, normalizedRect);

		src = buffers[horizontal];
		horizontal = 1 - horizontal;
	}

	Graphics::Blit(buffers[1 - horizontal], dest, normalizedRect);

	RenderTextureUtility::ReleaseTemporary(buffers[0]);
	RenderTextureUtility::ReleaseTemporary(buffers[1]);
}
