#pragma once
#include <glm/glm.hpp>

#include "light.h"
#include "entity.h"
#include "material.h"

class Pipeline;

class Shadows {
public:
	static void Initialize();
	static void AttachShadowTexture(Material material);

public:
	static void Resize(uint width, uint height);
	static const glm::mat4& GetWorldToShadowMatrix();
	static void Update(DirectionalLight light, Pipeline* pipeline, const std::vector<Entity>& entities);

private:
	Shadows();
};
