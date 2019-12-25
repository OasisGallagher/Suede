#include "time2internal.h"

#include "world.h"
#include "profiler.h"
#include "debug/debug.h"
#include "memory/refptr.h"
#include "frameeventqueue.h"

Time::Time() : Singleton2<Time>(new TimeInternal, t_delete<TimeInternal>) {}
float Time::GetTime() { return _suede_dinstance()->GetTime(); }
float Time::GetDeltaTime() { return _suede_dinstance()->GetDeltaTime(); }
float Time::GetFixedDeltaTime() { return _suede_dinstance()->GetFixedDeltaTime(); }
float Time::GetRealTimeSinceStartup() { return _suede_dinstance()->GetRealTimeSinceStartup(); }
uint Time::GetFrameCount() { return _suede_dinstance()->GetFrameCount(); }

TimeInternal::TimeInternal() : deltaTime_(0), frameCount_(0), lastFrameTimeStamp_(0) {
	World::frameEnter().subscribe(this, &TimeInternal::OnFrameEnter, (int)FrameEventQueue::Time);
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
