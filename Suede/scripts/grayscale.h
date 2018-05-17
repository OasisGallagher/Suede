#pragma once
#include "graphics.h"
#include "resources.h"
#include "imageeffect.h"

class Grayscale : public ImageEffect {
public:
	Grayscale() {
		material_ = NewMaterial();
		material_->SetShader(Resources::FindShader("grayscale"));
	}

public:
	virtual void OnRenderImage(RenderTexture src, RenderTexture dest) {
		Graphics::Blit(src, dest, material_);
	}

private:
	Material material_;
};
