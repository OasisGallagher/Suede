#pragma once
#include "posteffect.h"
#include "engine.h"

class Grayscale : public PostEffect {
public:
	virtual void OnRenderImage(RenderTexture src, RenderTexture dest) {
		World world = Engine::get()->world();
		Renderer renderer = dsp_cast<Renderer>(world->Create(ObjectTypeSurfaceRenderer));

		Shader shader = dsp_cast<Shader>(world->Create(ObjectTypeShader));
		shader->Load("shaders/grayscale");

		Material material = dsp_cast<Material>(world->Create(ObjectTypeMaterial));
		material->SetShader(shader);

		renderer->AddMaterial(material);
		Engine::get()->graphics()->Blit(src, dest, renderer);
	}
};
