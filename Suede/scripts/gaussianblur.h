#pragma once
#include "graphics.h"
#include "imageeffect.h"

class GaussianBlur : public ImageEffect {
public:
	GaussianBlur();

public:
	virtual void OnRenderImage(RenderTexture src, RenderTexture dest, const Rect& normalizedRect);

public:
	void SetAmount(uint value) { amount_ = value; }
	uint GetAmount() const { return amount_; }

private:
	uint amount_;
	Material material_;
};
