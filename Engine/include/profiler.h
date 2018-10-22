#pragma once
#include "enginedefines.h"
#include "tools/singleton.h"

class SUEDE_API Sample : public PimplIdiom {
	SUEDE_DECLARE_IMPLEMENTATION(Sample)

public:
	Sample();

public:
	void Start();
	void Restart();

	void Stop();

	void Reset();
	double GetElapsedSeconds() const;
};

class SUEDE_API Profiler : public Singleton2<Profiler> {
	friend class Singleton<Profiler>;
	SUEDE_DECLARE_IMPLEMENTATION(Profiler)

public:
	Sample* CreateSample();
	void ReleaseSample(Sample* value);

	uint64 GetTimeStamp();
	double TimeStampToSeconds(uint64 timeStamp);

private:
	Profiler();
};
