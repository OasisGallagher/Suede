#pragma once
#include <glm/glm.hpp>

#include "light.h"
#include "material.h"
#include "gameobject.h"

class Pipeline;

class Shadows {
public:
	Shadows(RenderTexture dest);

public:
	void Clear();
	RenderTexture GetShadowTexture();

	void Resize(uint width, uint height);
	const glm::mat4& GetWorldToShadowMatrix();
	void Update(Light light, Pipeline* pipeline);

private:
	glm::mat4 worldToShadowMatrix_;
	RenderTexture shadowDepthTexture_;
	Material directionalLightShadowMaterial_;
};
