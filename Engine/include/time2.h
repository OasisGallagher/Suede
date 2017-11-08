#pragma once
#include <memory>
#include "defines.h"

class ENGINE_EXPORT ITime {
public:
	virtual float GetDeltaTime() = 0;
	virtual float GetRealTimeSinceStartup() = 0;
	virtual int GetFrameCount() = 0;
	virtual void Update() = 0;
};

typedef std::shared_ptr<ITime> Time;
