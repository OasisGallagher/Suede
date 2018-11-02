#include "time2internal.h"

#include "profiler.h"
#include "debug/debug.h"
#include "memory/memory.h"

Time::Time() : Singleton2<Time>(MEMORY_NEW(TimeInternal), Memory::DeleteRaw<TimeInternal>) {}
float Time::GetTime() { return _suede_dinstance()->GetTime(); }
float Time::GetDeltaTime() { return _suede_dinstance()->GetDeltaTime(); }
float Time::GetRealTimeSinceStartup() { return _suede_dinstance()->GetRealTimeSinceStartup(); }
uint Time::GetFrameCount() { return _suede_dinstance()->GetFrameCount(); }

TimeInternal::TimeInternal() : deltaTime_(0), frameCount_(0), lastFrameTimeStamp_(0) {
	Engine::AddFrameEventListener(this);
}

void TimeInternal::OnFrameEnter() {
	uint64 timeStamp = Profiler::GetTimeStamp();
	time_ = (float)Profiler::TimeStampToSeconds(timeStamp);

	deltaTime_ = (float)Profiler::TimeStampToSeconds(timeStamp - lastFrameTimeStamp_);
	lastFrameTimeStamp_ = timeStamp;

	++frameCount_;
}

float TimeInternal::GetRealTimeSinceStartup() {
	return (float)Profiler::TimeStampToSeconds(Profiler::GetTimeStamp());
}
