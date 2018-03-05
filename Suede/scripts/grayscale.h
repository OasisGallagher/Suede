#pragma once
#include "graphics.h"
#include "resources.h"
#include "imageeffect.h"

class Grayscale : public ImageEffect {
public:
	Grayscale() {
		Shader shader = Resources::FindShader("buildin/shaders/blit");
		material_ = NewMaterial();
		material_->SetShader(shader);
	}

public:
	virtual void OnRenderImage(RenderTexture src, RenderTexture dest) {
		Graphics::Blit(src, dest, material_);
	}

private:
	Material material_;
};
