#pragma once
#include "material.h"
#include "renderer.h"

class SUEDE_API IGraphics {
public:
	virtual void Blit(RenderTexture src, RenderTexture dest, Material material) = 0;
};

SUEDE_DEFINE_OBJECT_POINTER(Graphics);
