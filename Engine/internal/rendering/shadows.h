#pragma once
#include <glm/glm.hpp>

#include "light.h"
#include "screen.h"
#include "entity.h"
#include "material.h"

class Pipeline;
class Framebuffer;

class Shadows : public ScreenSizeChangedListener {
public:
	static Shadows* Get();

public:
	virtual void OnScreenSizeChanged(uint width, uint height);

public:
	void Initialize();
	void AttachShadowTexture(Material material);

public:
	void Update(DirectionalLight light, Pipeline* pipeline, const std::vector<Entity>& entities);
	const glm::mat4& GetWorldToShadowMatrix() const { return worldToShadowMatrix; }

private:
	Shadows();
	~Shadows();

private:
	Framebuffer* fbDepth;

	glm::mat4 worldToShadowMatrix;
	RenderTexture shadowDepthTexture;
	Material directionalLightShadowMaterial;
};
