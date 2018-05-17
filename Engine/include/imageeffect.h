#pragma once
#include "texture.h"

class ImageEffect {
public:
	virtual ~ImageEffect() {}

public:
	virtual void OnRenderImage(RenderTexture src, RenderTexture dest) {

	}
};
