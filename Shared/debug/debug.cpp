#include "debug.h"

#include <fstream>

#include "stackwalker.h"
#include "../tools/string.h"

// declare an std::string variable named `_MsgVarName` and format it with message.
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
static LogReceiver* logReceiver;

bool Debug::Initialize() {
	return !!tracer.LoadModules();
}

void Debug::SetLogReceiver(LogReceiver* value) {
	logReceiver = value;
}

void Debug::Log(const char* format, ...) {
	DEF_VA_ARGS(msg, format);
	if (logReceiver != nullptr) {
		logReceiver->OnLogMessage(LogLevel::Debug, msg.c_str());
	}
}

void Debug::LogWarning(const char* format, ...) {
	DEF_VA_ARGS(msg, format);
	if (logReceiver != nullptr) {
		logReceiver->OnLogMessage(LogLevel::Warning, msg.c_str());
	}
}

void Debug::LogError(const char* format, ...) {
	DEF_VA_ARGS(msg, format);
	msg += "\n" + tracer.GetStackTrace(1, 7);

	if (logReceiver != nullptr) {
		logReceiver->OnLogMessage(LogLevel::Error, msg.c_str());
	}
}

#define SUEDE_DISABLE_VISUAL_STUDIO_OUTPUT

void Debug::Output(const char* format, ...) {
#ifndef SUEDE_DISABLE_VISUAL_STUDIO_OUTPUT
	va_list args;
	va_start(args, format);
	std::string msg = String::VFormat(format, args);
	msg += "\n";
	OutputDebugStringA(msg.c_str());
#endif
}

void Debug::Break() {
	__debugbreak();
}

const std::string& StackTracer::GetStackTrace(uint start, uint depth) {
	ln_ = 0;
	text_.clear();

	depth_ = depth;

	// skip this call & OnOutput.
	start_ = start + 2;

	ShowCallstack();

	// remove tailing newlines.
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
