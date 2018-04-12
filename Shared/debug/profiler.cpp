#include <stack>
#include <Windows.h>

#include "debug.h"
#include "profiler.h"

static double timeStampToSeconds = 1.0;
// TODO: multi-thread, one thread, one samples container.
static std::stack<uint64> samples;

void Profiler::Initialize() {
	Debug::LogWarning("TODO: Profiler multi-thread");

	LARGE_INTEGER frequency;
	if (QueryPerformanceFrequency(&frequency)) {
		timeStampToSeconds = 1.0 / frequency.QuadPart;
	}
	else {
		Debug::LogError("failed to initialize Profiler: %d.", GetLastError());
	}
}

void Profiler::StartSample() {
	samples.push(GetTimeStamp());
}

double Profiler::EndSample() {
	uint64 timeStamp = samples.top();
	samples.pop();
	return TimeStampToSeconds(GetTimeStamp() - timeStamp);
}

double Profiler::TimeStampToSeconds(uint64 timeStamp) {
	return timeStamp * timeStampToSeconds;
}

uint64 Profiler::GetTimeStamp() {
	LARGE_INTEGER qpc;
	if (QueryPerformanceCounter(&qpc)) {
		return qpc.QuadPart;
	}

	Debug::LogError("GetTimeStamp failed: %d.", GetLastError());
	return 0;
}
