#pragma once
#include "tools/singleton.h"

class SUEDE_API Time : public Singleton2<Time> {
public:
	/**
	 * @brief the time in seconds at the beginning of this frame.
	 */
	virtual float GetTime() = 0;

	/**
	 * @brief the time in seconds it took to complete the last frame.
	 */
	virtual float GetDeltaTime() = 0;

	/**
	 * @brief the real time in seconds since the engine started.
	 */
	virtual float GetRealTimeSinceStartup() = 0;

	/**
	 * @brief the total number of frames that have passed.
	 */
	virtual uint GetFrameCount() = 0;
};
