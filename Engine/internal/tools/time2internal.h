#pragma once
#include "time2.h"

#include "engine.h"
#include "enginedefines.h"

class TimeInternal : public FrameEventListener {
public:
	TimeInternal();

public:
	float GetTime() { return time_; }
	float GetDeltaTime() { return deltaTime_; }

	float GetRealTimeSinceStartup();
	uint GetFrameCount() { return frameCount_; }

public:
	void OnFrameEnter();
	int GetFrameEventQueue() { return FrameEventQueueTime; }

private:
	float time_;
	float deltaTime_;

	uint frameCount_;
	uint64 lastFrameTimeStamp_;
};