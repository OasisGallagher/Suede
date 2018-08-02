#pragma once
#include "profiler.h"

#include "engine.h"
#include "containers/freelist.h"

class SampleInternal : public Sample {
public:
	SampleInternal() { Reset(); }
	friend class free_list<SampleInternal>;

public:
	virtual void Start();
	virtual void Restart();

	virtual void Stop();

	virtual void Reset();
	virtual double GetElapsedSeconds() const;

private:
	bool started_;
	uint64 elapsed_;
	uint64 timeStamp_;
};

class ProfilerInternal : public Profiler, public FrameEventListener {
public:
	ProfilerInternal();
	~ProfilerInternal();

public:
	virtual Sample* CreateSample();
	virtual void ReleaseSample(Sample* value);

	virtual uint64 GetTimeStamp();
	virtual double TimeStampToSeconds(uint64 timeStamp);

public:
	virtual void OnFrameEnter();
	virtual int GetFrameEventQueue() { return FrameEventQueueProfiler; }

private:
	enum {
		MaxProfilterSamples = 1024,
	};

	typedef free_list<SampleInternal> SampleContainer;
	SampleContainer samples_;
	double timeStampToSeconds_;
};