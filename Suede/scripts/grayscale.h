#pragma once
#include "graphics.h"
#include "posteffect.h"

class Grayscale : public PostEffect {
public:
	virtual void OnRenderImage(RenderTexture src, RenderTexture dest) {
		World world = Engine::GetWorld();

		Shader shader = dsp_cast<Shader>(world->Create(ObjectTypeShader));
		shader->Load("shaders/grayscale");

		Material material = dsp_cast<Material>(world->Create(ObjectTypeMaterial));
		material->SetShader(shader);

		Graphics::Blit(src, dest, material);
	}
};
