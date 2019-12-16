#include "resources.h"
#include "gaussianblur.h"

SUEDE_DEFINE_COMPONENT(GaussianBlur, ImageEffect)

void GaussianBlur::Awake() {
	amount_.reset(5, 1, 10);
	material_ = new Material();
	material_->SetName("Gaussian Blur");
	material_->SetShader(Shader::Find("gaussianblur"));
}

void GaussianBlur::OnRenderImage(RenderTexture* src, RenderTexture* dest, const Rect& normalizedRect) {
	int horizontal = 1;
	int width = src->GetWidth(), height = src->GetHeight();

	// Temporary render textures.
	RenderTexture* buffers[] = {
		RenderTexture::GetTemporary(RenderTextureFormat::RgbF, width, height),
		RenderTexture::GetTemporary(RenderTextureFormat::RgbF, width, height)
	};

	for (int i = 0; i < amount_.get_value(); ++i) {
		material_->SetBool("horizontal", horizontal != 0);
		Graphics::Blit(src, buffers[horizontal], material_.get(), normalizedRect);

		src = buffers[horizontal];
		horizontal = 1 - horizontal;
	}

	Graphics::Blit(buffers[1 - horizontal], dest, normalizedRect);

	RenderTexture::ReleaseTemporary(buffers[0]);
	RenderTexture::ReleaseTemporary(buffers[1]);
}
