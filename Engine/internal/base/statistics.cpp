#include "statistics.h"

#include "time2.h"
#include "frameeventqueue.h"

#define lastFrameStats	stats_[0]
#define thisFrameStats	stats_[1]
#define FPS_REFRESH_TIME	0.2f

Statistics::Statistics() : frameRate_(0), timeCounter_(0), frameCounter_(0) {
	memset(&lastFrameStats, 0, sizeof(lastFrameStats));
	memset(&thisFrameStats, 0, sizeof(thisFrameStats));

	World::frameEnter().subscribe(this, &Statistics::OnFrameEnter, (int)FrameEventQueue::Statistics);
}

Statistics::~Statistics() {
	World::frameEnter().unsubscribe(this);
}

void Statistics::OnFrameEnter() {
	lastFrameStats = thisFrameStats;
	memset(&thisFrameStats, 0, sizeof(thisFrameStats));

	if (timeCounter_ < FPS_REFRESH_TIME) {
		timeCounter_ += Time::GetDeltaTime();
		frameCounter_++;
	}
	else {
		frameRate_ = (float)frameCounter_ / timeCounter_;
		frameCounter_ = 0;
		timeCounter_ = 0.0f;
	}
}

void Statistics::SetScriptElapsed(double value) {
	thisFrameStats.scriptElapsed = value;
}

void Statistics::SetCullingElapsed(double value) {
	thisFrameStats.cullingElapsed = value;
}

void Statistics::SetRenderingElapsed(double value) {
	thisFrameStats.renderingElapsed = value;
}

const FrameStatistics& Statistics::GetFrameStatistics() {
	lastFrameStats.frameRate = frameRate_;
	return lastFrameStats;
}

void Statistics::AddTriangles(uint n) {
	thisFrameStats.ntriangles += n;
}

void Statistics::AddDrawcalls(uint n) {
	thisFrameStats.ndrawcalls += n;
}
