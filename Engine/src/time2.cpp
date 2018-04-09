#include "time2.h"
#include "debug/debug.h"
#include "debug/profiler.h"

static uint frames;
static uint64 ticks;
static double deltaTime;

void Time::Update() {
	uint64 now = Profiler::GetTicks();
	deltaTime = (now - ticks) * Profiler::GetSecondsPerTick();
	ticks = now;

	++frames;
}

float Time::GetDeltaTime() {
	return (float)deltaTime;
}

float Time::GetRealTimeSinceStartup() {
	return float(Profiler::GetTicks() * Profiler::GetSecondsPerTick());
}

uint Time::GetFrameCount() {
	return frames;
}
