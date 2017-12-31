#include <stack>
#include <ctime>
#include <cstdarg>

#include "debug.h"

static ILogReceiver* logReceiver;
static std::stack<std::string> samples;
static char buffer[512];

#define FORMAT_BUFFER(format)	*buffer = 0; \
	va_list ap; va_start(ap, format); \
	vsnprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), format, ap); va_end(ap)

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

void Debug::StartSample(const char* text) {
	clock_t now = clock();
	samples.push(std::to_string(now) + "#" + text);
}

void Debug::EndSample() {
	std::string samp = samples.top();
	samples.pop();

	std::string::size_type pos = samp.find('#');
	samp[pos] = 0;

	clock_t elapsed = clock() - atol(samp.c_str());
	Debug::Log("\"%s\" costs %.2f seconds.", samp.c_str() + pos + 1, ((float)elapsed / CLOCKS_PER_SEC));
}
