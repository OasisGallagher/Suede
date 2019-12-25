#pragma once
#include "defines.h"
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

class SUEDE_API Profiler : private Singleton2<Profiler> {
	friend class Singleton<Profiler>;
	SUEDE_DECLARE_IMPLEMENTATION(Profiler)

public:
	static Sample* CreateSample();
	static void ReleaseSample(Sample* value);

	static uint64 GetTimeStamp();
	static double TimeStampToSeconds(uint64 timeStamp);

private:
	Profiler();
};
