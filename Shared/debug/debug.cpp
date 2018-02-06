#include <stack>
#include <ctime>

#include "debug.h"
#include "stackwalker.h"

static LogReceiver* logReceiver;
static std::stack<clock_t> samples;
static char buffer[512];

class StackTracer : public StackWalker {
public:
	StackTracer(uint skipLineCount, uint depth = 3) : skipLineCount_(skipLineCount + 2), depth_(depth) {
		// +2 for OnOutput and GetStackTrace.
	}

public:
	const std::string& GetStackTrace() {
		text_.clear();
		lineCount_ = 0;
		ShowCallstack();
		return text_;
	}

protected:
	virtual void OnOutput(const char* text) {
		if (++lineCount_ > skipLineCount_ && lineCount_ <= depth_) {
			//StackWalker::OnOutput(text);
			text_ += text;
		}
	}

private:
	uint depth_;
	uint lineCount_;
	uint skipLineCount_;
	std::string text_;
};

#define FORMAT_BUFFER(format)	*buffer = 0; \
	va_list ap; va_start(ap, format); \
	vsnprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), format, ap); va_end(ap)

void Debug::SetLogReceiver(LogReceiver* value) {
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
		StackTracer tracer(1);
		FORMAT_BUFFER(format);
		std::string text = std::string(buffer) + "\n" + tracer.GetStackTrace();
		logReceiver->OnLogMessage(LogLevelError, text.c_str());
	}
}

void Debug::Break() {
	__debugbreak();
}

void Debug::StartSample() {
	samples.push(clock());
}

float Debug::EndSample() {
	clock_t samp = samples.top();
	samples.pop();

	return float(clock() - samp) / CLOCKS_PER_SEC;
}
