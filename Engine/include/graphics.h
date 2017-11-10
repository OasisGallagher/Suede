#pragma once
#include "material.h"
#include "renderer.h"

class ENGINE_EXPORT IGraphics {
public:
	virtual void Blit(RenderTexture src, RenderTexture dest, Material material) = 0;
};

typedef std::shared_ptr<IGraphics> Graphics;
