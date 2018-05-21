#pragma once
#include "material.h"
#include "renderer.h"

class SUEDE_API Graphics {
public:
	static void Draw(Mesh mesh, Material material);

	static void Blit(RenderTexture src, RenderTexture dest);
	static void Blit(RenderTexture src, RenderTexture dest, Material material);

private:
	Graphics();
};
