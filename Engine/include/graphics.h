#pragma once
#include "mesh.h"
#include "material.h"
#include "renderer.h"
#include "subsystem.h"

#include "tools/enum.h"

BETTER_ENUM(ShadingMode, int,
	Shaded,
	Wireframe
)

class SUEDE_API Graphics : public Subsystem {
	SUEDE_DECLARE_IMPLEMENTATION(Graphics)

public:
	enum {
		SystemType = SubsystemType::Graphics,
	};

public:
	Graphics();

public:
	void SetShadingMode(ShadingMode value);
	ShadingMode GetShadingMode();

	void SetAmbientOcclusionEnabled(bool value);
	bool GetAmbientOcclusionEnabled();

	void SetRenderTarget(std::vector<uint>& colorBuffers, uint depthBuffer);

	void Draw(Mesh* mesh, Material* material);

	void Blit(Texture* src, RenderTexture* dest);
	void Blit(Texture* src, RenderTexture* dest, const Rect& rect);
	void Blit(Texture* src, RenderTexture* dest, const Rect& srcRect, const Rect& destRect);

	void Blit(Texture* src, RenderTexture* dest, Material* material);
	void Blit(Texture* src, RenderTexture* dest, Material* material, const Rect& rect);
	void Blit(Texture* src, RenderTexture* dest, Material* material, const Rect& srcRect, const Rect& destRect);

public:
	virtual void Awake();
};
