#include "statisticsinternal.h"

#include "time2.h"
#include "memory/memory.h"

#undef _dptr
#define _dptr()	((StatisticsInternal*)d_)
Statistics::Statistics() : Singleton2<Statistics>(MEMORY_NEW(StatisticsInternal)) {}
void Statistics::AddTriangles(uint n) { _dptr()->AddTriangles(n); }
void Statistics::AddDrawcalls(uint n) { _dptr()->AddDrawcalls(n); }
uint Statistics::GetTriangles() { return _dptr()->GetTriangles(); }
uint Statistics::GetDrawcalls() { return _dptr()->GetDrawcalls(); }
float Statistics::GetFrameRate() { return _dptr()->GetFrameRate(); }
void Statistics::SetCullingElapsed(double value) { _dptr()->SetCullingElapsed(value); }
void Statistics::SetRenderingElapsed(double value) { _dptr()->SetRenderingElapsed(value); }
double Statistics::GetCullingElapsed() { return _dptr()->GetCullingElapsed(); }
double Statistics::GetRenderingElapsed() { return _dptr()->GetRenderingElapsed(); }

#define lastFrameStats	stats_[0]
#define thisFrameStats	stats_[1]
#define FPS_REFRESH_TIME	0.2f

StatisticsInternal::StatisticsInternal() : frameRate_(0), timeCounter_(0), frameCounter_(0) {
	memset(&lastFrameStats, 0, sizeof(lastFrameStats));
	memset(&thisFrameStats, 0, sizeof(thisFrameStats));

	Engine::instance()->AddFrameEventListener(this);
}

void StatisticsInternal::OnFrameEnter() {
	lastFrameStats = thisFrameStats;
	thisFrameStats.Reset();

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

void StatisticsInternal::FrameStats::Reset() {
	ndrawcalls = nvertices = ntriangles = 0;
}
