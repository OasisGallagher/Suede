#pragma once
#include "defines.h"
#include "tools/singleton.h"

class SUEDE_API Time : private Singleton2<Time> {
	friend class Singleton<Time>;
	SUEDE_DECLARE_IMPLEMENTATION(Time)

public:
	/**
	 * @brief the time in seconds at the beginning of this frame.
	 */
	static float GetTime();

	/**
	 * @brief the time in seconds it took to complete the last frame.
	 */
	static float GetDeltaTime();

	/**
	 * @brief the interval in seconds at which physics and other fixed frame rate updates are performed.
	 */
	static float GetFixedDeltaTime();

	/**
	 * @brief the real time in seconds since the engine started.
	 */
	static float GetRealTimeSinceStartup();

	/**
	 * @brief the total number of frames that have passed.
	 */
	static uint GetFrameCount();

private:
	Time();
};
