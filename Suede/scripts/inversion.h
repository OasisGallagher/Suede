#pragma once
#include "graphics.h"
#include "imageeffect.h"

class Inversion : public ImageEffect {
public:
	Inversion() {
		material_ = NewMaterial();
		Shader shader = Resources::FindShader("inversion");
		material_->SetShader(shader);
	}

public:
	virtual void OnRenderImage(RenderTexture src, RenderTexture dest) {
		Graphics::Blit(src, dest, material_);
	}

private:
	Material material_;
};
