#include <ctime>
#include "time2.h"

#define CLOCKS_PER_SEC_F	float(CLOCKS_PER_SEC)

static uint frames;
static clock_t clocks;
static float deltaTime;

void Time::Update() {
	clock_t now = clock();
	deltaTime = (now - clocks) / CLOCKS_PER_SEC_F;
	clocks = now;

	++frames;
}

float Time::GetDeltaTime() {
	return deltaTime;
}

float Time::GetRealTimeSinceStartup() {
	return clock() / CLOCKS_PER_SEC_F;
}

uint Time::GetFrameCount() {
	return frames;
}
