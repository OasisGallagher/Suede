#include "time2.h"
#include "debug/debug.h"
#include "debug/profiler.h"

static uint frameCount;
static uint64 lastFrameTimeStamp;
static double deltaTime;

void Time::Update() {
	uint64 timeStamp = Profiler::GetTimeStamp();
	deltaTime = Profiler::TimeStampToSeconds(timeStamp - lastFrameTimeStamp);
	lastFrameTimeStamp = timeStamp;

	++frameCount;
}

float Time::GetDeltaTime() {
	return (float)deltaTime;
}

float Time::GetRealTimeSinceStartup() {
	return float(Profiler::TimeStampToSeconds(Profiler::GetTimeStamp()));
}

uint Time::GetFrameCount() {
	return frameCount;
}
