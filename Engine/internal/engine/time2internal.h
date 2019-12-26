#pragma once
#include "time2.h"
#include "subsysteminternal.h"

class TimeInternal : public SubsystemInternal {
public:
	float GetTime() { return time_; }
	float GetDeltaTime() { return deltaTime_; }

	// SUEDE TODO: Fixed delta time.
	float GetFixedDeltaTime() { return deltaTime_; }

	float GetRealTimeSinceStartup();
	uint GetFrameCount() { return frameCount_; }

	void Update(float deltaTime);

private:
	float time_ = 0;
	float deltaTime_ = 0;
	double timeStampToSeconds_ = 1.0;

	uint frameCount_ = 0;
	uint64 lastFrameTimeStamp_ = 0;
};
