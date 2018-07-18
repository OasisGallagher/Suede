#include "time2.h"
#include "statistics.h"

#define lastFrameStats	stats_[0]
#define thisFrameStats	stats_[1]
#define FPS_REFRESH_TIME	0.5f

Statistics::Statistics() : frameRate_(0), timeCounter_(0), frameCounter_(0) {
	Engine::get()->AddFrameEventListener(this);
}

void Statistics::OnFrameEnter() {
	lastFrameStats = thisFrameStats;
	memset(&thisFrameStats, 0, sizeof(FrameStats));

	if (timeCounter_ < FPS_REFRESH_TIME) {
		timeCounter_ += Time::get()->GetDeltaTime();
		frameCounter_++;
	}
	else {
		frameRate_ = (float)frameCounter_ / timeCounter_;
		frameCounter_ = 0;
		timeCounter_ = 0.0f;
	}
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
	return frameRate_;
}
