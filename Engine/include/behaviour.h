#pragma once
#include "component.h"

class SUEDE_API Behaviour : public IComponent {
	SUEDE_DECLARE_COMPONENT()

public:
	Behaviour();

public:
	virtual void Awake() {}
	virtual void Update() {}
	virtual void OnRenderImage(RenderTexture src, RenderTexture dest, const Rect& normalizedRect);
};
