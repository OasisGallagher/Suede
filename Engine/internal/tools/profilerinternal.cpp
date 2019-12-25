#include "profilerinternal.h"

#include <Windows.h>

#include "debug/debug.h"
#include "containers/freelist.h"

Sample::Sample() : PimplIdiom(new SampleInternal, t_delete<SampleInternal>) {}
void Sample::Start() { _suede_dptr()->Start(); }
void Sample::Restart() { _suede_dptr()->Restart(); }
void Sample::Stop() { _suede_dptr()->Stop(); }
void Sample::Reset() { _suede_dptr()->Reset(); }
double Sample::GetElapsedSeconds() const { return _suede_dptr()->GetElapsedSeconds(); }

void SampleInternal::Start() {
	started_ = true;
	timeStamp_ = Profiler::GetTimeStamp();
}

void SampleInternal::Restart() {
	Reset();
	Start();
}

void SampleInternal::Stop() {
	started_ = false;
	elapsed_ += (Profiler::GetTimeStamp() - timeStamp_);
}

void SampleInternal::Reset() {
	started_ = false;
	elapsed_ = timeStamp_ = 0;
}

double SampleInternal::GetElapsedSeconds() const {
	if (started_) {
		Debug::LogError("call Stop() first.");
		return 0.0;
	}

	return Profiler::TimeStampToSeconds(elapsed_);
}

static std::mutex mutex_;
static free_list<Sample> samples_(1024);

Sample* Profiler::CreateSample() {
	std::lock_guard<std::mutex> lock(mutex_);
	return samples_.spawn();
}

void Profiler::ReleaseSample(Sample* sample) {
	sample->Reset();

	std::lock_guard<std::mutex> lock(mutex_);
	samples_.recycle(sample);
}

double Profiler::TimeStampToSeconds(uint64 timeStamp) {
	static double timeStampToSeconds = []() {
		LARGE_INTEGER frequency;
		if (QueryPerformanceFrequency(&frequency)) {
			return 1.0 / frequency.QuadPart;
		}

		Debug::LogError("failed to initialize Profiler: %d.", GetLastError());
		return 1.0;
	}();

	return timeStamp * timeStampToSeconds;
}

uint64 Profiler::GetTimeStamp() {
	LARGE_INTEGER qpc;
	if (QueryPerformanceCounter(&qpc)) {
		return qpc.QuadPart;
	}

	Debug::LogError("GetTimeStamp failed: %d.", GetLastError());
	return 0;
}
