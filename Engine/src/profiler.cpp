#include <Windows.h>

#include "profiler.h"
#include "debug/debug.h"

Profiler::Profiler() : samples_(MaxProfilterSamples) {
	LARGE_INTEGER frequency;
	if (QueryPerformanceFrequency(&frequency)) {
		timeStampToSeconds_ = 1.0 / frequency.QuadPart;
	}
	else {
		timeStampToSeconds_ = 1.0;
		Debug::LogError("failed to initialize Profiler: %d.", GetLastError());
	}

	Engine::get()->AddFrameEventListener(this);
}

void Profiler::OnFrameEnter() {
	//for (SampleContainer::iterator ite = samples_.begin(); ite != samples_.end(); ++ite) {
	//	(*ite)->Reset();
	//}
}

Sample* Profiler::CreateSample() {
	return samples_.spawn();
}

void Profiler::ReleaseSample(Sample* sample) {
	sample->Reset();
	samples_.recycle(sample);
}

double Profiler::TimeStampToSeconds(uint64 timeStamp) {
	return timeStamp * timeStampToSeconds_;
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
	timeStamp_ = Profiler::get()->GetTimeStamp();
}

void Sample::Restart() {
	Reset();
	Start();
}

void Sample::Stop() {
	started_ = false;
	elapsed_ += (Profiler::get()->GetTimeStamp() - timeStamp_);
}

void Sample::Reset() {
	started_ = false;
	elapsed_ = timeStamp_ = 0;
}

double Sample::GetElapsedSeconds() const {
	if (started_) {
		Debug::LogError("call Stop() first.");
		return 0.0;
	}

	return Profiler::get()->TimeStampToSeconds(elapsed_);
}
