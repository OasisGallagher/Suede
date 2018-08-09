#pragma once
#include <glm/glm.hpp>

#include "light.h"
#include "entity.h"
#include "material.h"
#include "tools/singleton.h"

class Pipeline;

class Shadows : public Singleton<Shadows> {
	friend class Singleton<Shadows>;

public:
	void Clear();
	RenderTexture GetShadowTexture();

	void Resize(uint width, uint height);
	const glm::mat4& GetWorldToShadowMatrix();
	void Update(DirectionalLight light, Pipeline* pipeline);

private:
	Shadows();

private:
	glm::mat4 worldToShadowMatrix_;
	RenderTexture shadowDepthTexture_;
	Material directionalLightShadowMaterial_;
};
