#pragma once
#include "time2.h"

#include "engine.h"
#include "enginedefines.h"

class TimeInternal : public Time, public FrameEventListener {
public:
	TimeInternal();

public:
	virtual float GetTime() { return time_; }
	virtual float GetDeltaTime() { return deltaTime_; }

	virtual float GetRealTimeSinceStartup();
	virtual uint GetFrameCount() { return frameCount_; }

public:
	virtual void OnFrameEnter();
	virtual int GetFrameEventQueue() { return FrameEventQueueTime; }

private:
	float time_;
	float deltaTime_;

	uint frameCount_;
	uint64 lastFrameTimeStamp_;
};