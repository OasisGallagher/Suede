#pragma once
#include "defines.h"

class SUEDE_API Time {
public:
	static float GetDeltaTime();
	static float GetRealTimeSinceStartup();

	static uint GetFrameCount();

private:
	friend class Engine;
	static void Update();

private:
	Time();
};
