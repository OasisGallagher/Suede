#pragma once
#include "../shareddefines.h"

class SHARED_API Profiler {
public:
	static void Initialize();

public:
	static void StartSample();
	static double EndSample();

	static uint64 GetTimeStamp();
	static double TimeStampToSeconds(uint64 timeStamp);
};
