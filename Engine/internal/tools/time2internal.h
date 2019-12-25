#pragma once
#include "time2.h"

class TimeInternal {
public:
	TimeInternal();

public:
	float GetTime() { return time_; }
	float GetDeltaTime() { return deltaTime_; }

	// SUEDE TODO: Fixed delta time.
	float GetFixedDeltaTime() { return deltaTime_; }

	float GetRealTimeSinceStartup();
	uint GetFrameCount() { return frameCount_; }

private:
	void OnFrameEnter();

private:
	float time_;
	float deltaTime_;

	uint frameCount_;
	uint64 lastFrameTimeStamp_;
};
