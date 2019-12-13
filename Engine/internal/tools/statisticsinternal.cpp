#include "statisticsinternal.h"

#include "time2.h"
#include "memory/refptr.h"
#include "frameeventqueue.h"

Statistics::Statistics() : Singleton2<Statistics>(new StatisticsInternal, t_delete<StatisticsInternal>) {}
void Statistics::AddTriangles(uint n) { _suede_dinstance()->AddTriangles(n); }
void Statistics::AddDrawcalls(uint n) { _suede_dinstance()->AddDrawcalls(n); }
uint Statistics::GetTriangles() { return _suede_dinstance()->GetTriangles(); }
uint Statistics::GetDrawcalls() { return _suede_dinstance()->GetDrawcalls(); }
float Statistics::GetFrameRate() { return _suede_dinstance()->GetFrameRate(); }
void Statistics::SetScriptElapsed(double value) { _suede_dinstance()->SetScriptElapsed(value); }
double Statistics::GetScriptElapsed() { return _suede_dinstance()->GetScriptElapsed(); }
void Statistics::SetCullingElapsed(double value) { _suede_dinstance()->SetCullingElapsed(value); }
double Statistics::GetCullingElapsed() { return _suede_dinstance()->GetCullingElapsed(); }
void Statistics::SetRenderingElapsed(double value) { _suede_dinstance()->SetRenderingElapsed(value); }
double Statistics::GetRenderingElapsed() { return _suede_dinstance()->GetRenderingElapsed(); }

#define lastFrameStats	stats_[0]
#define thisFrameStats	stats_[1]
#define FPS_REFRESH_TIME	0.2f

StatisticsInternal::StatisticsInternal() : frameRate_(0), timeCounter_(0), frameCounter_(0) {
	memset(&lastFrameStats, 0, sizeof(lastFrameStats));
	memset(&thisFrameStats, 0, sizeof(thisFrameStats));

	Engine::frameEnter.subscribe(this, &StatisticsInternal::OnFrameEnter, (int)FrameEventQueue::Statistics);
}

void StatisticsInternal::OnFrameEnter() {
	lastFrameStats = thisFrameStats;
	thisFrameStats.Reset();

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

void StatisticsInternal::SetScriptElapsed(double value) {
	thisFrameStats.scriptElapsed = value;
}

double StatisticsInternal::GetScriptElapsed() {
	return thisFrameStats.scriptElapsed;
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
