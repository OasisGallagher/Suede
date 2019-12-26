#pragma once
#include "defines.h"
#include "subsystem.h"

class SUEDE_API Time : public Subsystem {
	SUEDE_DECLARE_IMPLEMENTATION(Time)

public:
	enum {
		SystemType = SubsystemType::Time,
	};

public:
	Time();

public:
	static uint64 GetTimeStamp();
	static double TimeStampToSeconds(uint64 timeStamp);

public:
	/**
	 * @brief the time in seconds at the beginning of this frame.
	 */
	float GetTime();

	/**
	 * @brief the time in seconds it took to complete the last frame.
	 */
	float GetDeltaTime();

	/**
	 * @brief the interval in seconds at which physics and other fixed frame rate updates are performed.
	 */
	float GetFixedDeltaTime();

	/**
	 * @brief the real time in seconds since the engine started.
	 */
	float GetRealTimeSinceStartup();

	/**
	 * @brief the total number of frames that have passed.
	 */
	uint GetFrameCount();

public:
	virtual void Update(float deltaTime);
};
