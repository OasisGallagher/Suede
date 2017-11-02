#pragma once
#include "texture.h"
#include "renderer.h"

class ENGINE_EXPORT IGraphics {
public:
	virtual void Blit(RenderTexture src, RenderTexture dest, Renderer renderer) = 0;
};

typedef std::shared_ptr<IGraphics> Graphics;
