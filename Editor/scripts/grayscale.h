#pragma once
#include "graphics.h"
#include "resources.h"
#include "imageeffect.h"

class Grayscale : public ImageEffect {
public:
	Grayscale() {
		material_ = NewMaterial();
		material_->SetShader(Resources::instance()->FindShader("grayscale"));
	}

public:
	virtual void OnRenderImage(RenderTexture src, RenderTexture dest, const Rect& normalizedRect) {
		Graphics::instance()->Blit(src, dest, material_, normalizedRect);
	}

private:
	Material material_;
};