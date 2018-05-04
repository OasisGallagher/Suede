#include <stack>

#include "debug.h"
#include "stackwalker.h"

static LogReceiver* logReceiver;

class StackTracer : public StackWalker {
public:
	StackTracer();

public:
	const std::string& GetStackTrace(uint start, uint depth = 3);

protected:
	virtual void OnOutput(const char* text);

private:
	std::string text_;
	uint ln_, depth_, start_;
};

static StackTracer tracer;

#define MAX_LOG_LENGTH	512
#define FORMAT_BUFFER(format, bufname)	\
	char bufname[MAX_LOG_LENGTH]; \
	va_list ap; \
	va_start(ap, format); \
	vsnprintf(bufname, sizeof(bufname) / sizeof(bufname[0]), format, ap); \
	va_end(ap)

void Debug::SetLogReceiver(LogReceiver* value) {
	logReceiver = value; 
}

void Debug::Log(const char* format, ...) {
	if (logReceiver != nullptr) {
		FORMAT_BUFFER(format, buffer);
		logReceiver->OnLogMessage(LogLevelDebug, buffer);
	}
}

void Debug::LogWarning(const char* format, ...) {
	if (logReceiver != nullptr) {
		FORMAT_BUFFER(format, buffer);
		logReceiver->OnLogMessage(LogLevelWarning, buffer);
	}
}

void Debug::LogError(const char* format, ...) {
	if (logReceiver != nullptr) {
		FORMAT_BUFFER(format, buffer);
		std::string text = std::string(buffer) + "\n" + tracer.GetStackTrace(1, 5);
		logReceiver->OnLogMessage(LogLevelError, text.c_str());
	}
}

void Debug::Output(const char* format, ...) {
	FORMAT_BUFFER(format, buffer);
	OutputDebugStringA(buffer);
}

void Debug::Break() {
	__debugbreak();
}

StackTracer::StackTracer() {
	LoadModules();
}

const std::string & StackTracer::GetStackTrace(uint start, uint depth) {
	text_.clear();
	ln_ = 0;

	depth_ = depth;

	// skip this call & OnOutput.
	start_ = start + 2;

	ShowCallstack();
	return text_;
}

void StackTracer::OnOutput(const char* text) {
	if (++ln_ > start_ && ln_ <= depth_) {
		text_ += text;
	}
}
