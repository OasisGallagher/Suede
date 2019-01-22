#pragma once
#include "component.h"

class SUEDE_API IBehaviour : public IComponent {
	SUEDE_DECLARE_COMPONENT()

public:
	IBehaviour();

public:
	virtual void Awake() {}
	virtual void Update() {}
	virtual void OnRenderImage(RenderTexture src, RenderTexture dest, const Rect& normalizedRect);
};

SUEDE_DEFINE_OBJECT_POINTER(Behaviour)
