#include <Windows.h>

#include <OpenThreads/Mutex>
#include <OpenThreads/ScopedLock>

#include "debug.h"
#include "profiler.h"

OpenThreads::Mutex mutex;
#define LockSampleContainerInScole()	OpenThreads::ScopedLock<OpenThreads::Mutex> lock(mutex)

static double timeStampToSeconds = 1.0;

#define MAX_PROFILTER_SAMPLES	1024
static free_list<Sample> samples(MAX_PROFILTER_SAMPLES);

void Profiler::Initialize() {
	LARGE_INTEGER frequency;
	if (QueryPerformanceFrequency(&frequency)) {
		timeStampToSeconds = 1.0 / frequency.QuadPart;
	}
	else {
		Debug::LogError("failed to initialize Profiler: %d.", GetLastError());
	}
}

void Profiler::OnFrameEnter() {

}

void Profiler::OnFrameExit() {

}

Sample* Profiler::CreateSample() {
	LockSampleContainerInScole();
	return samples.spawn();
}

void Profiler::ReleaseSample(Sample* sample) {
	LockSampleContainerInScole();
	sample->Clear();
	samples.recycle(sample);
}

double Profiler::TimeStampToSeconds(uint64 timeStamp) {
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

void Sample::Start() {
	started_ = true;
	timeStamp_ = Profiler::GetTimeStamp();
}

void Sample::Restart() {
	Clear();
	Start();
}

void Sample::Stop() {
	started_ = false;
	elapsed_ += (Profiler::GetTimeStamp() - timeStamp_);
}

void Sample::Clear() {
	started_ = false;
	elapsed_ = timeStamp_ = 0;
}

double Sample::GetElapsedSeconds() const {
	if (started_) {
		Debug::LogError("call Stop() first.");
		return 0.0;
	}

	return Profiler::TimeStampToSeconds(elapsed_);
}