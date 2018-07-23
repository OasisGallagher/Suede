#pragma once
#include "graphics.h"
#include "imageeffect.h"

class GaussianBlur : public ImageEffect {
public:
	GaussianBlur() : amount_(5){
		material_ = NewMaterial();
		material_->SetShader(Resources::get()->FindShader("gaussianblur"));
	}

public:
	virtual void OnRenderImage(RenderTexture src, RenderTexture dest, const Rect& normalizedRect) {
		bool first = 1, horizontal = 1;
		int rtW = src->GetWidth(), rtH = src->GetHeight();

		// temporary render texture.
		RenderTexture buffers[2];
		buffers[0] = NewRenderTexture();
		buffers[0]->Create(RenderTextureFormatRgbHDR, rtW, rtH);

		buffers[1] = NewRenderTexture();
		buffers[1]->Create(RenderTextureFormatRgbHDR, rtW, rtH);

		for (int i = 0; i < amount_; ++i) {
			material_->SetBool("horizontal", horizontal);
			Graphics::Blit((first != 0) ? src : buffers[1 - horizontal], buffers[horizontal], material_, normalizedRect);

			horizontal = !horizontal;
			first = false;
		}

		Graphics::Blit(buffers[1 - horizontal], dest, normalizedRect);
	}

public:
	void SetAmount(uint value) { amount_ = value; }
	uint GetAmount() const { return amount_; }

private:
	uint amount_;
	Material material_;
};
