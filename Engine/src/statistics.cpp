#include "time2.h"
#include "engine.h"
#include "statistics.h"

static float frameRate;
static float timeCounter;
static uint frameCounter;

struct FrameStats {
	uint ndrawcalls;
	uint nvertices;
	uint ntriangles;
} stats[2];

#define FPS_REFRESH_TIME	0.5f
FrameStats& lastFrameStats = stats[0];
FrameStats& thisFrameStats = stats[1];

Statistics::Statistics() {
	Engine::AddFrameEventListener(this);
}

void Statistics::OnFrameEnter() {
	lastFrameStats = thisFrameStats;
	memset(&thisFrameStats, 0, sizeof(FrameStats));

	if (timeCounter < FPS_REFRESH_TIME) {
		timeCounter += Time::GetDeltaTime();
		frameCounter++;
	}
	else {
		frameRate = (float)frameCounter / timeCounter;
		frameCounter = 0;
		timeCounter = 0.0f;
	}
}

void Statistics::OnFrameLeave() {
}

void Statistics::Initialize() {
	static Statistics instance;
}

void Statistics::AddTriangles(uint n) {
	thisFrameStats.ntriangles += n;
}

void Statistics::AddDrawcalls(uint n) {
	thisFrameStats.ndrawcalls += n;
}

uint Statistics::GetTriangles() {
	return lastFrameStats.ntriangles;
}

uint Statistics::GetDrawcalls() {
	return lastFrameStats.ndrawcalls;
}

float Statistics::GetFrameRate() {
	return frameRate;
}
