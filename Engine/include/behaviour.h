#pragma once
#include "component.h"

class SUEDE_API Behaviour : public Component {
	SUEDE_DECLARE_COMPONENT()

public:
	Behaviour();

public:
	virtual void Awake() {}
	virtual void Update() {}
	virtual void OnRenderImage(RenderTexture* src, RenderTexture* dest, const Rect& normalizedRect);
};
