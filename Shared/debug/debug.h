#pragma once
#include <string>
#include "../types.h"

enum LogLevel {
	LogLevelDebug,
	LogLevelWarning,
	LogLevelError,
	LogLevelFatal,
};

class SUEDE_API LogReceiver {
public:
	virtual void OnLogMessage(LogLevel level, const char* message) = 0;
};

class SUEDE_API Debug {
public:
	static void Log(const char* format, ...);
	static void LogWarning(const char* format, ...);
	static void LogError(const char* format, ...);

	static void Break();
	static void Output(const char* format, ...);
	static void SetLogReceiver(LogReceiver* value);

private:
	friend class Engine;
	static void Initialize();

private:
	Debug();
};
