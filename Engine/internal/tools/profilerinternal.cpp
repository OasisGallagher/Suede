#include "profilerinternal.h"

#include <Windows.h>
#include "debug/debug.h"

ProfilerInternal::ProfilerInternal() : samples_(MaxProfilterSamples) {
	LARGE_INTEGER frequency;
	if (QueryPerformanceFrequency(&frequency)) {
		timeStampToSeconds_ = 1.0 / frequency.QuadPart;
	}
	else {
		timeStampToSeconds_ = 1.0;
		Debug::LogError("failed to initialize ProfilerInternal: %d.", GetLastError());
	}

	Engine::instance()->AddFrameEventListener(this);
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
	samples_.recycle((SampleInternal*)sample);
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
	timeStamp_ = ProfilerInternal::instance()->GetTimeStamp();
}

void SampleInternal::Restart() {
	Reset();
	Start();
}

void SampleInternal::Stop() {
	started_ = false;
	elapsed_ += (ProfilerInternal::instance()->GetTimeStamp() - timeStamp_);
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

	return ProfilerInternal::instance()->TimeStampToSeconds(elapsed_);
}
