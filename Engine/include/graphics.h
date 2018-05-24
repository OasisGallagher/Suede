#pragma once
#include "material.h"
#include "renderer.h"

class SUEDE_API Graphics {
public:
	static void Draw(Mesh mesh, Material material);

	static void Blit(Texture src, RenderTexture dest);
	static void Blit(Texture src, RenderTexture dest, const Rect& rect);
	static void Blit(Texture src, RenderTexture dest, const Rect& srcRect, const Rect& destRect);

	static void Blit(Texture src, RenderTexture dest, Material material);
	static void Blit(Texture src, RenderTexture dest, Material material, const Rect& rect);
	static void Blit(Texture src, RenderTexture dest, Material material, const Rect& srcRect, const Rect& destRect);

private:
	Graphics();
};
