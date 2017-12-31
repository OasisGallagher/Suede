#pragma once
#include <string>
#include "../shareddefines.h"

enum LogLevel {
	LogLevelDebug,
	LogLevelWarning,
	LogLevelError,
	LogLevelFatal,
};

class ILogReceiver {
public:
	virtual void OnLogMessage(LogLevel level, const char* message) = 0;
};

class SHARED_API Debug {
public:
	static void Log(const char* format, ...);
	static void LogWarning(const char* format, ...);
	static void LogError(const char* format, ...);

	static void StartSample(const char* text);
	static void EndSample();

	static void SetLogReceiver(ILogReceiver* value);

private:
	Debug();
};
