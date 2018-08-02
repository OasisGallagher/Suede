#include "resources.h"
#include "gaussianblur.h"

GaussianBlur::GaussianBlur() : amount_(5) {
	material_ = NewMaterial();
	material_->SetShader(Resources::instance()->FindShader("gaussianblur"));
}

void GaussianBlur::OnRenderImage(RenderTexture src, RenderTexture dest, const Rect& normalizedRect) {
	int horizontal = 1;
	int width = src->GetWidth(), height = src->GetHeight();

	// temporary render texture.
	RenderTexture buffers[2];
	buffers[0] = NewRenderTexture();
	buffers[0]->Create(RenderTextureFormatRgbHDR, width, height);

	buffers[1] = NewRenderTexture();
	buffers[1]->Create(RenderTextureFormatRgbHDR, width, height);

	for (int i = 0; i < amount_; ++i) {
		material_->SetBool("horizontal", horizontal != 0);
		Graphics::instance()->Blit(src, buffers[horizontal], material_, normalizedRect);

		src = buffers[horizontal];
		horizontal = 1 - horizontal;
	}

	Graphics::instance()->Blit(buffers[1 - horizontal], dest, normalizedRect);
}
