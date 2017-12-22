#include <cstdarg>

#include "math2.h"
#include "debug.h"

static ILogReceiver* logReceiver;

static char buffer[512];

#define FORMAT_BUFFER(format)	*buffer = 0; \
	va_list ap; va_start(ap, format); \
	vsnprintf(buffer, CountOf(buffer), format, ap); va_end(ap)

void Debug::SetLogReceiver(ILogReceiver* value) {
	logReceiver = value;
}

void Debug::Log(const char* format, ...) {
	if (logReceiver != nullptr) {
		FORMAT_BUFFER(format);
		logReceiver->OnLogMessage(LogLevelDebug, buffer);
	}
}

void Debug::LogWarning(const char* format, ...) {
	if (logReceiver != nullptr) {
		FORMAT_BUFFER(format);
		logReceiver->OnLogMessage(LogLevelWarning, buffer);
	}
}

void Debug::LogError(const char* format, ...) {
	if (logReceiver != nullptr) {
		FORMAT_BUFFER(format);
		logReceiver->OnLogMessage(LogLevelError, buffer);
	}
}
