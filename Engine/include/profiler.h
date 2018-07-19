#pragma once
#include "engine.h"
#include "tools/singleton.h"
#include "containers/freelist.h"

class SUEDE_API Sample {
public:
	void Start();
	void Restart();

	void Stop();

	void Reset();
	double GetElapsedSeconds() const;

private:
	/**
	 * use Profiler::get()->CreateSample() to create.
	 */
	Sample() { Reset(); }
	friend class free_list<Sample>;

private:
	bool started_;
	uint64 elapsed_;
	uint64 timeStamp_;
};

class SUEDE_API Profiler : public Singleton<Profiler>, public FrameEventListener {
	friend class Singleton<Profiler>;

public:
	Sample* CreateSample();
	void ReleaseSample(Sample* value);

	uint64 GetTimeStamp();
	double TimeStampToSeconds(uint64 timeStamp);

public:
	virtual void OnFrameEnter();
	virtual int GetFrameEventQueue() { return FrameEventQueueProfiler; }

private:
	Profiler();

private:
	enum {
		MaxProfilterSamples = 1024,
	};

	typedef free_list<Sample> SampleContainer;
	SampleContainer samples_;
	double timeStampToSeconds_;
};
