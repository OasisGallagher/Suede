#pragma once
#include "mesh.h"
#include "material.h"
#include "renderer.h"
#include "tools/enum.h"

BETTER_ENUM(ShadingMode, int,
	Shaded,
	Wireframe
)

class SUEDE_API Graphics {
public:
	static void SetShadingMode(ShadingMode value);
	static ShadingMode GetShadingMode();

	static void SetAmbientOcclusionEnabled(bool value);
	static bool GetAmbientOcclusionEnabled();

	static void SetRenderTarget(std::vector<uint>& colorBuffers, uint depthBuffer);

	static void Draw(Mesh* mesh, Material* material);

	static void Blit(Texture* src, RenderTexture* dest);
	static void Blit(Texture* src, RenderTexture* dest, const Rect& rect);
	static void Blit(Texture* src, RenderTexture* dest, const Rect& srcRect, const Rect& destRect);

	static void Blit(Texture* src, RenderTexture* dest, Material* material);
	static void Blit(Texture* src, RenderTexture* dest, Material* material, const Rect& rect);
	static void Blit(Texture* src, RenderTexture* dest, Material* material, const Rect& srcRect, const Rect& destRect);
};
