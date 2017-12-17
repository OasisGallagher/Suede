#pragma once
#include "graphics.h"
#include "imageeffect.h"

class Grayscale : public ImageEffect {
public:
	virtual void OnRenderImage(RenderTexture src, RenderTexture dest) {
		Shader shader = NewShader();
		if (shader->Load("buildin/shaders/blit")) {
			Material material = NewMaterial();
			material->SetShader(shader);
			Graphics::Blit(src, dest, material);
		}
	}
};
