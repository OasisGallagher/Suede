#pragma once
#include "tools/singleton.h"

class SUEDE_API Sample {
public:
	virtual void Start() = 0;
	virtual void Restart() = 0;

	virtual void Stop() = 0;

	virtual void Reset() = 0;
	virtual double GetElapsedSeconds() const = 0;
};

class SUEDE_API Profiler : public Singleton2<Profiler> {
public:
	virtual Sample* CreateSample() = 0;
	virtual void ReleaseSample(Sample* value) = 0;

	virtual uint64 GetTimeStamp() = 0;
	virtual double TimeStampToSeconds(uint64 timeStamp) = 0;
};
