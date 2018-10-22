#pragma once
#include "profiler.h"

#include "engine.h"
#include "containers/freelist.h"

class SampleInternal {
public:
	SampleInternal() { Reset(); }

public:
	void Start();
	void Restart();

	void Stop();

	void Reset();
	double GetElapsedSeconds() const;

private:
	bool started_;
	uint64 elapsed_;
	uint64 timeStamp_;
};

class ProfilerInternal : public FrameEventListener {
public:
	ProfilerInternal();
	~ProfilerInternal();

public:
	Sample* CreateSample();
	void ReleaseSample(Sample* value);

	uint64 GetTimeStamp();
	double TimeStampToSeconds(uint64 timeStamp);

public:
	void OnFrameEnter();
	int GetFrameEventQueue() { return FrameEventQueueProfiler; }

private:
	enum {
		MaxProfilterSamples = 1024,
	};

	typedef free_list<Sample> SampleContainer;
	SampleContainer samples_;
	double timeStampToSeconds_;
};