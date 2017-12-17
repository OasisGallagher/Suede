#pragma once
#include "graphics.h"
#include "imageeffect.h"

class Inversion : public ImageEffect {
public:
	virtual void OnRenderImage(RenderTexture src, RenderTexture dest) {
		World world = WorldInstance();

		Shader shader = NewShader();
		shader->Load("shaders/inversion");

		Material material = NewMaterial();
		material->SetShader(shader);

		Graphics::Blit(src, dest, material);
	}
};
