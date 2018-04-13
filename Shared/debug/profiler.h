#pragma once
#include "../shareddefines.h"
#include "../containers/freelist.h"

class SHARED_API Sample {
public:
	void Start();
	void Restart();

	void Stop();

	void Clear();
	double GetElapsedSeconds() const;

private:
	/**
	 * use Profiler::CreateSample() to create.
	 */
	Sample() { Clear(); }
	friend class free_list<Sample>;

private:
	bool started_;
	uint64 elapsed_;
	uint64 timeStamp_;
};

class SHARED_API Profiler {
public:
	static void Initialize();
	static void OnFrameEnter();
	static void OnFrameExit();

public:
	static Sample* CreateSample();
	static void ReleaseSample(Sample* value);

	static uint64 GetTimeStamp();
	static double TimeStampToSeconds(uint64 timeStamp);
};
