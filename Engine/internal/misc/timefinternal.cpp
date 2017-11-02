#include "timefinternal.h"
#include "internal/memory/memory.h"

#define ClockToSecond(clocks)	(float(clocks) / CLOCKS_PER_SEC)

Time timeInstance(Memory::Create<TimeInternal>());

TimeInternal::TimeInternal() :clocks_(0), frames_(0), deltaTime_(0) {
}

void TimeInternal::Update() {
	clock_t now = clock();
	deltaTime_ = ClockToSecond(now - clocks_);
	clocks_ = now;

	++frames_;
}

float TimeInternal::GetDeltaTime() {
	return deltaTime_;
}

float TimeInternal::GetRealTimeSinceStartup() {
	return ClockToSecond(clock());
}

int TimeInternal::GetFrameCount() {
	return frames_;
}
