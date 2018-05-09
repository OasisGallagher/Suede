#pragma once
#include "graphics.h"
#include "imageeffect.h"

class Inversion : public ImageEffect {
public:
	Inversion() {
		material_ = NewMaterial();
		material_->SetShader(Resources::FindShader("inversion"));
	}

public:
	virtual void OnRenderImage(RenderTexture src, RenderTexture dest) {
		Graphics::Blit(src, dest, material_);
	}

private:
	Material material_;
};
