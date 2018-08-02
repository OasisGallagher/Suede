#pragma once
#include "material.h"
#include "renderer.h"
#include "tools/enum.h"
#include "tools/singleton.h"

BETTER_ENUM(ShadingMode, int,
	Shaded,
	Wireframe
)

class SUEDE_API Graphics : public Singleton2<Graphics> {
public:
	virtual void SetShadingMode(ShadingMode value) = 0;
	virtual ShadingMode GetShadingMode() = 0;

	virtual void Draw(Mesh mesh, Material material) = 0;

	virtual void Blit(Texture src, RenderTexture dest) = 0;
	virtual void Blit(Texture src, RenderTexture dest, const Rect& rect) = 0;
	virtual void Blit(Texture src, RenderTexture dest, const Rect& srcRect, const Rect& destRect) = 0;

	virtual void Blit(Texture src, RenderTexture dest, Material material) = 0;
	virtual void Blit(Texture src, RenderTexture dest, Material material, const Rect& rect) = 0;
	virtual void Blit(Texture src, RenderTexture dest, Material material, const Rect& srcRect, const Rect& destRect) = 0;
};
