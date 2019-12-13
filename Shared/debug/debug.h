#pragma once
#include <string>
#include <cassert>
#include "../types.h"

#include "../tools/event.h"
#include "../tools/singleton.h"

#define SUEDE_NOARG
#define SUEDE_VERIFY_INDEX(index, bounds, returns) \
	if (index >= bounds) { Debug::LogError("index out of range"); return returns; } else (void)0

#define SUEDE_ASSERT(expression)	assert(expression)

enum class LogLevel {
	Debug,
	Warning,
	Error,
};

class SUEDE_API Debug {
public:
	static void Log(const char* format, ...);
	static void LogWarning(const char* format, ...);
	static void LogError(const char* format, ...);

	static void Break();
	static void Output(const char* format, ...);

	static event<LogLevel, const char*> logReceived;

public:
	static bool Initialize();

private:
	Debug();
};
