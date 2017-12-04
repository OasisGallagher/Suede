#pragma once
#include "graphics.h"
#include "posteffect.h"

class Grayscale : public PostEffect {
public:
	virtual void OnRenderImage(RenderTexture src, RenderTexture dest) {
		Shader shader = NewShader();
		shader->Load("shaders/grayscale");

		Material material = NewMaterial();
		material->SetShader(shader);

		Graphics::Blit(src, dest, material);
	}
};
