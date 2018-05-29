#include <stack>

#include "debug.h"
#include "stackwalker.h"

static LogReceiver* logReceiver;

class StackTracer : public StackWalker {
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
#define VA_FORMAT(format, bufname) \
	va_list _Ap; \
	va_start(_Ap, format); \
	int _L = vsnprintf(bufname, sizeof(bufname) / sizeof(bufname[0]), format, _Ap); \
	va_end(_Ap)

#define FORMAT_BUFFER(format, bufname)	\
	char bufname[MAX_LOG_LENGTH]; \
	VA_FORMAT(format, bufname)

#define FORMAT_LINE_BUFFER(format, bufname)	\
	char bufname[MAX_LOG_LENGTH - 1]; \
	VA_FORMAT(format, bufname); \
	bufname[_L++] = '\n', bufname[_L] = 0

void Debug::Initialize() {
	tracer.LoadModules();
}

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
	FORMAT_LINE_BUFFER(format, buffer);
	OutputDebugStringA(buffer);
}

void Debug::Break() {
	__debugbreak();
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
