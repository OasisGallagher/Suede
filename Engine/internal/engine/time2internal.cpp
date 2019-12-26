#include "time2internal.h"

#include <Windows.h>

#include "engine.h"
#include "profiler.h"
#include "debug/debug.h"
#include "frameevents.h"
#include "memory/refptr.h"

Time::Time() : Subsystem(new TimeInternal) {}
void Time::Update(float deltaTime) { _suede_dptr()->Update(deltaTime); }
float Time::GetTime() { return _suede_dptr()->GetTime(); }
float Time::GetDeltaTime() { return _suede_dptr()->GetDeltaTime(); }
float Time::GetFixedDeltaTime() { return _suede_dptr()->GetFixedDeltaTime(); }
float Time::GetRealTimeSinceStartup() { return _suede_dptr()->GetRealTimeSinceStartup(); }
uint Time::GetFrameCount() { return _suede_dptr()->GetFrameCount(); }

uint64 Time::GetTimeStamp() {
	LARGE_INTEGER qpc;
	if (QueryPerformanceCounter(&qpc)) {
		return qpc.QuadPart;
	}

	Debug::LogError("GetTimeStamp failed: %d.", GetLastError());
	return 0;
}

double Time::TimeStampToSeconds(uint64 timeStamp) {
	static double timeStampToSeconds = []() {
		LARGE_INTEGER frequency;
		if (QueryPerformanceFrequency(&frequency)) {
			return 1.0 / frequency.QuadPart;
		}

		Debug::LogError("failed to initialize Profiler: %d.", GetLastError());
		return 1.0;
	}();

	return timeStamp * timeStampToSeconds;
}

void TimeInternal::Update(float deltaTime) {
	uint64 timeStamp = Time::GetTimeStamp();
	time_ = (float)Time::TimeStampToSeconds(timeStamp);

	deltaTime_ = (float)Time::TimeStampToSeconds(timeStamp - lastFrameTimeStamp_);
	lastFrameTimeStamp_ = timeStamp;

	++frameCount_;
}

float TimeInternal::GetRealTimeSinceStartup() {
	return (float)Time::TimeStampToSeconds(Time::GetTimeStamp());
}
