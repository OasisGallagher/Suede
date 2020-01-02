#pragma once
#include <cassert>
#include "../types.h"

#define SUEDE_ASSERT(expression)	assert(expression)

enum class LogLevel {
	Debug,
	Warning,
	Error,
};

class SUEDE_API Debug {
public:
	class Logger {
	public:
		virtual ~Logger() {}
		virtual void OnLogMessageReceived(LogLevel level, const char* message) = 0;
	};

public:
	static void Log(const char* format, ...);
	static void LogWarning(const char* format, ...);
	static void LogError(const char* format, ...);

	static void Break();
	static void OutputToConsole(const char* format, ...);

	static Logger* GetLogger();
	static void SetLogger(Logger* value);

private:
	Debug();
};
