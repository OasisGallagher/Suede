#pragma once
#include "graphics.h"
#include "resources.h"
#include "imageeffect.h"

class Grayscale : public ImageEffect {
public:
	virtual void OnRenderImage(RenderTexture src, RenderTexture dest) {
		Shader shader = Resources::FindShader("buildin/shaders/grayscale");
		Material material = NewMaterial();
		material->SetShader(shader);
		Graphics::Blit(src, dest, material);
	}
};
