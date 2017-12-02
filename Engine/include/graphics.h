#pragma once
#include "material.h"
#include "renderer.h"

class SUEDE_API Graphics {
public:
	static void Blit(RenderTexture src, RenderTexture dest, Material material);

private:
	Graphics();
};
