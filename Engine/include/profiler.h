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

class SUEDE_API Profiler : private singleton2<Profiler> {
	friend class singleton<Profiler>;
	SUEDE_DECLARE_IMPLEMENTATION(Profiler)

public:
	static Sample* CreateSample();
	static void ReleaseSample(Sample* value);

	static uint64 GetTimeStamp();
	static double TimeStampToSeconds(uint64 timeStamp);

private:
	Profiler();
};

#define PROFILER_RECORD(var, ...) \
	double var = 0;  \
	if (true) {\
		uint64 start = Profiler::GetTimeStamp(); \
		__VA_ARGS__; \
		var = Profiler::TimeStampToSeconds(Profiler::GetTimeStamp() - start); \
	} else (void)0
