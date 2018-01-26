#pragma once
#include <string>
#include "../shareddefines.h"

enum LogLevel {
	LogLevelDebug,
	LogLevelWarning,
	LogLevelError,
	LogLevelFatal,
};

class LogReceiver {
public:
	virtual void OnLogMessage(LogLevel level, const char* message) = 0;
};

class SHARED_API Debug {
public:
	static void Log(const char* format, ...);
	static void LogWarning(const char* format, ...);
	static void LogError(const char* format, ...);

	static void StartSample();
	static float EndSample();

	static void SetLogReceiver(LogReceiver* value);

private:
	Debug();
};
