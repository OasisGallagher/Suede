#pragma once
#include "enginedefines.h"

class SUEDE_API Time {
public:
	/**
	 * @brief the time in seconds it took to complete the last frame.
	 */
	static float GetDeltaTime();
	static float GetRealTimeSinceStartup();

	static uint GetFrameCount();

private:
	friend class Engine;
	static void Update();

private:
	Time();
};
