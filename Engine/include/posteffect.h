#pragma once
#include "texture.h"

class PostEffect {
public:
	virtual void OnRenderImage(RenderTexture src, RenderTexture dest) = 0;
};
