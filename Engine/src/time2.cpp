#include "time2.h"
#include "profiler.h"
#include "debug/debug.h"

Time::Time() : deltaTime_(0), frameCount_(0), lastFrameTimeStamp_(0) {
	Engine::get()->AddFrameEventListener(this);
}

void Time::OnFrameEnter() {
	uint64 timeStamp = Profiler::get()->GetTimeStamp();
	time_ = (float)Profiler::get()->TimeStampToSeconds(timeStamp);

	deltaTime_ = (float)Profiler::get()->TimeStampToSeconds(timeStamp - lastFrameTimeStamp_);
	lastFrameTimeStamp_ = timeStamp;

	++frameCount_;
}

float Time::GetRealTimeSinceStartup() {
	return (float)Profiler::get()->TimeStampToSeconds(Profiler::get()->GetTimeStamp());
}

uint Time::GetFrameCount() {
	return frameCount_;
}
