#include "time2.h"
#include "debug/debug.h"
#include "debug/profiler.h"

static uint frameCount;
static uint64 lastFrameTicks;
static double deltaTime;

void Time::Update() {
	uint64 ticks = Profiler::GetTicks();
	deltaTime = (ticks - lastFrameTicks) * Profiler::GetSecondsPerTick();
	lastFrameTicks = ticks;

	++frameCount;
}

float Time::GetDeltaTime() {
	return (float)deltaTime;
}

float Time::GetRealTimeSinceStartup() {
	return float(Profiler::GetTicks() * Profiler::GetSecondsPerTick());
}

uint Time::GetFrameCount() {
	return frameCount;
}
