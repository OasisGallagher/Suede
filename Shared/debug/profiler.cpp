#include <stack>
#include <Windows.h>

#include "debug.h"
#include "profiler.h"

static double secondsPerTick = 1.0;
static std::stack<uint64> samples;

void Profiler::Initialize() {
	LARGE_INTEGER frequency;
	if (QueryPerformanceFrequency(&frequency)) {
		secondsPerTick = 1.0 / frequency.QuadPart;
	}
	else {
		Debug::LogError("initialize Time failed: %d.", GetLastError());
	}
}

void Profiler::StartSample() {
	samples.push(GetTicks());
}

double Profiler::EndSample() {
	uint64 samp = samples.top();
	samples.pop();

	return (GetTicks() - samp) * GetSecondsPerTick();
}

double Profiler::GetSecondsPerTick() {
	return secondsPerTick;
}

uint64 Profiler::GetTicks() {
	LARGE_INTEGER qpc;
	if (QueryPerformanceCounter(&qpc)) {
		return qpc.QuadPart;
	}

	Debug::LogError("GetTicks failed: %d.", GetLastError());
	return 0;
}
