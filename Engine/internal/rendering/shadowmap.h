#pragma once
#include "light.h"
#include "material.h"
#include "gameobject.h"

class Pipeline;

class ShadowMap {
public:
	ShadowMap(RenderTexture* target);

public:
	void Clear();

	void Resize(uint width, uint height);
	const Matrix4& GetWorldToShadowMatrix();
	void Update(Light* light);

	Material* GetMaterial() { return directionalLightShadowMaterial_.get(); }
	RenderTexture* GetTargetTexture() { return shadowDepthTexture_.get(); }

private:
	Matrix4 worldToShadowMatrix_;
	ref_ptr<RenderTexture> shadowDepthTexture_;
	ref_ptr<Material> directionalLightShadowMaterial_;
};
