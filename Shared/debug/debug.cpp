#include "debug.h"

#include "stackwalker.h"
#include "../tools/string.h"

/**
 * Declare an std::string variable named `_MsgVarName` and format it with message.
 */
#define DEF_VA_ARGS(_MsgVarName, _Format) \
	va_list _Args; va_start(_Args, _Format); \
	std::string _MsgVarName = String::VFormat(_Format, _Args); \
	va_end(_Args)

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
static Debug::Logger* logger;

void Debug::Log(const char* format, ...) {
	if (logger != nullptr) {
		DEF_VA_ARGS(msg, format);
		logger->OnLogMessageReceived(LogLevel::Debug, msg.c_str());
	}
}

void Debug::LogWarning(const char* format, ...) {
	if (logger != nullptr) {
		DEF_VA_ARGS(msg, format);
		logger->OnLogMessageReceived(LogLevel::Warning, msg.c_str());
	}
}

void Debug::LogError(const char* format, ...) {
	if (logger != nullptr) {
		DEF_VA_ARGS(msg, format);
		msg += "\n" + tracer.GetStackTrace(1, 30);
		logger->OnLogMessageReceived(LogLevel::Error, msg.c_str());
	}
}

#define SUEDE_DISABLE_VISUAL_STUDIO_OUTPUT

void Debug::OutputToConsole(const char* format, ...) {
#ifndef SUEDE_DISABLE_VISUAL_STUDIO_OUTPUT
	DEF_VA_ARGS(msg, format);
	OutputDebugStringA((msg += "\n").c_str());
#endif
}

Debug::Logger* Debug::GetLogger() { return logger; }

void Debug::SetLogger(Logger* value) {
    if (logger != value && (logger = value) != nullptr) {
        tracer.LoadModules();
    }
}

void Debug::Break() {
	__debugbreak();
}

const std::string& StackTracer::GetStackTrace(uint start, uint depth) {
	ln_ = 0;
	text_.clear();

	depth_ = depth;

	// Skip this call & OnOutput.
	start_ = start + 2;

	ShowCallstack();

	// Remove tailing newlines.
	for (; !text_.empty() && text_.back() == '\n'; ) {
		text_.pop_back();
	}

	return text_;
}

void StackTracer::OnOutput(const char* text) {
	if (++ln_ > start_ && ln_ <= depth_) {
		text_ += text;
	}
}
