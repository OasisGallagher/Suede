#pragma once
#include "defines.h"

class SUEDE_API ITime {
public:
	virtual float GetDeltaTime() = 0;
	virtual float GetRealTimeSinceStartup() = 0;

	virtual uint GetFrameCount() = 0;

	virtual void Update() = 0;
};

SUEDE_DEFINE_OBJECT_POINTER(Time);
