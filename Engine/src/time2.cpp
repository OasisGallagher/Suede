#include "time2.h"
#include "debug/debug.h"
#include "debug/profiler.h"

void Time::Update() {
	uint64 timeStamp = Profiler::get()->GetTimeStamp();
	deltaTime_ = Profiler::get()->TimeStampToSeconds(timeStamp - lastFrameTimeStamp_);
	lastFrameTimeStamp_ = timeStamp;

	++frameCount_;
}

float Time::GetDeltaTime() {
	return (float)deltaTime_;
}

float Time::GetRealTimeSinceStartup() {
	return float(Profiler::get()->TimeStampToSeconds(Profiler::get()->GetTimeStamp()));
}

uint Time::GetFrameCount() {
	return frameCount_;
}
