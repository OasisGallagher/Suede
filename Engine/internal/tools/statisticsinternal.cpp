#include "statisticsinternal.h"

#include "time2.h"

#define lastFrameStats	stats_[0]
#define thisFrameStats	stats_[1]
#define FPS_REFRESH_TIME	0.2f

StatisticsInternal::StatisticsInternal() : frameRate_(0), timeCounter_(0), frameCounter_(0) {
	Engine::instance()->AddFrameEventListener(this);
}

void StatisticsInternal::OnFrameEnter() {
	lastFrameStats = thisFrameStats;
	memset(&thisFrameStats, 0, sizeof(thisFrameStats));

	if (timeCounter_ < FPS_REFRESH_TIME) {
		timeCounter_ += Time::instance()->GetDeltaTime();
		frameCounter_++;
	}
	else {
		frameRate_ = (float)frameCounter_ / timeCounter_;
		frameCounter_ = 0;
		timeCounter_ = 0.0f;
	}
}

void StatisticsInternal::SetCullingElapsed(double value) {
	thisFrameStats.cullingElapsed = value;
}

void StatisticsInternal::SetRenderingElapsed(double value) {
	thisFrameStats.renderingElapsed = value;
}

double StatisticsInternal::GetCullingElapsed() {
	return lastFrameStats.cullingElapsed;
}

double StatisticsInternal::GetRenderingElapsed() {
	return lastFrameStats.renderingElapsed;
}

void StatisticsInternal::AddTriangles(uint n) {
	thisFrameStats.ntriangles += n;
}

void StatisticsInternal::AddDrawcalls(uint n) {
	thisFrameStats.ndrawcalls += n;
}

uint StatisticsInternal::GetTriangles() {
	return lastFrameStats.ntriangles;
}

uint StatisticsInternal::GetDrawcalls() {
	return lastFrameStats.ndrawcalls;
}

float StatisticsInternal::GetFrameRate() {
	return frameRate_;
}
