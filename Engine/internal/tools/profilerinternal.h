#pragma once
#include "profiler.h"

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
