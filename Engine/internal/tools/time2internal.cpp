#include "time2internal.h"

#include "profiler.h"
#include "debug/debug.h"
#include "memory/memory.h"

#undef _dptr
#define _dptr()	((TimeInternal*)d_)

Time::Time() : Singleton2<Time>(MEMORY_NEW(TimeInternal)) {}
float Time::GetTime() { return _dptr()->GetTime(); }
float Time::GetDeltaTime() { return _dptr()->GetDeltaTime(); }
float Time::GetRealTimeSinceStartup() { return _dptr()->GetRealTimeSinceStartup(); }
uint Time::GetFrameCount() { return _dptr()->GetFrameCount(); }

TimeInternal::TimeInternal() : deltaTime_(0), frameCount_(0), lastFrameTimeStamp_(0) {
	Engine::instance()->AddFrameEventListener(this);
}

void TimeInternal::OnFrameEnter() {
	uint64 timeStamp = Profiler::instance()->GetTimeStamp();
	time_ = (float)Profiler::instance()->TimeStampToSeconds(timeStamp);

	deltaTime_ = (float)Profiler::instance()->TimeStampToSeconds(timeStamp - lastFrameTimeStamp_);
	lastFrameTimeStamp_ = timeStamp;

	++frameCount_;
}

float TimeInternal::GetRealTimeSinceStartup() {
	return (float)Profiler::instance()->TimeStampToSeconds(Profiler::instance()->GetTimeStamp());
}
