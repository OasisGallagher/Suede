#pragma once
#include "texture.h"
#include "behaviour.h"

class SUEDE_API ImageEffect : public Behaviour {
	SUEDE_DECLARE_COMPONENT()

public:
	virtual void OnRenderImage(RenderTexture src, RenderTexture dest, const Rect& normalizedRect) {}
};
