#include "profilerinternal.h"

#include <Windows.h>
#include "debug/debug.h"

Sample::Sample() : PimplIdiom(MEMORY_NEW(SampleInternal), Memory::DeleteRaw<SampleInternal>) {}
void Sample::Start() { _suede_dptr()->Start(); }
void Sample::Restart() { _suede_dptr()->Restart(); }
void Sample::Stop() { _suede_dptr()->Stop(); }
void Sample::Reset() { _suede_dptr()->Reset(); }
double Sample::GetElapsedSeconds() const { return _suede_dptr()->GetElapsedSeconds(); }

Profiler::Profiler() : singleton2<Profiler>(MEMORY_NEW(ProfilerInternal), Memory::DeleteRaw<ProfilerInternal>) {}
Sample* Profiler::CreateSample() { return _suede_dinstance()->CreateSample(); }
void Profiler::ReleaseSample(Sample* value) { _suede_dinstance()->ReleaseSample(value); }
uint64 Profiler::GetTimeStamp() { return _suede_dinstance()->GetTimeStamp(); }
double Profiler::TimeStampToSeconds(uint64 timeStamp) { return _suede_dinstance()->TimeStampToSeconds(timeStamp); }

ProfilerInternal::ProfilerInternal() : samples_(MaxProfilerSamples) {
	LARGE_INTEGER frequency;
	if (QueryPerformanceFrequency(&frequency)) {
		timeStampToSeconds_ = 1.0 / frequency.QuadPart;
	}
	else {
		timeStampToSeconds_ = 1.0;
		Debug::LogError("failed to initialize Profiler: %d.", GetLastError());
	}

	Engine::AddFrameEventListener(this);
}

ProfilerInternal::~ProfilerInternal() {
}

void ProfilerInternal::OnFrameEnter() {
	//for (SampleContainer::iterator ite = samples_.begin(); ite != samples_.end(); ++ite) {
	//	(*ite)->Reset();
	//}
}

Sample* ProfilerInternal::CreateSample() {
	return samples_.spawn();
}

void ProfilerInternal::ReleaseSample(Sample* sample) {
	sample->Reset();
	samples_.recycle(sample);
}

double ProfilerInternal::TimeStampToSeconds(uint64 timeStamp) {
	return timeStamp * timeStampToSeconds_;
}

uint64 ProfilerInternal::GetTimeStamp() {
	LARGE_INTEGER qpc;
	if (QueryPerformanceCounter(&qpc)) {
		return qpc.QuadPart;
	}

	Debug::LogError("GetTimeStamp failed: %d.", GetLastError());
	return 0;
}

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
