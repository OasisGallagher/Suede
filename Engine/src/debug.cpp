#include <cstdarg>

#include "math2.h"
#include "debug.h"

static ILogReceiver* logReceiver;

static char buffer[512];

#define FORMAT(format)	*buffer = 0; \
	va_list ap; va_start(ap, format); \
	vsnprintf(buffer, CountOf(buffer), format, ap); va_end(ap)

void Debug::SetLogReceiver(ILogReceiver* value) {
	logReceiver = value;
}

void Debug::Log(const char* format, ...) {
	if (logReceiver != nullptr) {
		FORMAT(format);
		logReceiver->OnLogMessage(LogLevelDebug, buffer);
	}
}

void Debug::LogWarning(const char* format, ...) {
	if (logReceiver != nullptr) {
		FORMAT(format);
		logReceiver->OnLogMessage(LogLevelWarning, buffer);
	}
}

void Debug::LogError(const char* format, ...) {
	if (logReceiver != nullptr) {
		FORMAT(format);
		logReceiver->OnLogMessage(LogLevelError, buffer);
	}
}
