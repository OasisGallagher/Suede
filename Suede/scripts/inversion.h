#pragma once
#include "graphics.h"
#include "imageeffect.h"

class Inversion : public ImageEffect {
public:
	Inversion() {
		material_ = NewMaterial();
		material_->SetShader(Resources::get()->FindShader("inversion"));
	}

public:
	virtual void OnRenderImage(RenderTexture src, RenderTexture dest, const Rect& normalizedRect) {
		Graphics::Blit(src, dest, material_, normalizedRect);
	}

private:
	Material material_;
};
