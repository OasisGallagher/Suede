#pragma once
#include "tools/singleton.h"

class SUEDE_API Sample {
public:
	Sample();

public:
	void Start();
	void Restart();

	void Stop();

	void Reset();
	double GetElapsedSeconds() const;

private:
	void* d_;
};

class SUEDE_API Profiler : public Singleton2<Profiler> {
	friend class Singleton2<Profiler>;

public:
	Sample* CreateSample();
	void ReleaseSample(Sample* value);

	uint64 GetTimeStamp();
	double TimeStampToSeconds(uint64 timeStamp);

private:
	Profiler();
};
