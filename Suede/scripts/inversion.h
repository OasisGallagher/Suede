#pragma once
#include "posteffect.h"
#include "engine.h"

class Inversion : public PostEffect {
public:
	virtual void OnRenderImage(RenderTexture src, RenderTexture dest) {
		World world = Engine::get()->world();

		Shader shader = dsp_cast<Shader>(world->Create(ObjectTypeShader));
		shader->Load("shaders/inversion");

		Material material = dsp_cast<Material>(world->Create(ObjectTypeMaterial));
		material->SetShader(shader);

		Engine::get()->graphics()->Blit(src, dest, material);
	}
};
