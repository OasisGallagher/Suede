#pragma once
#include <stack>
#include <string>

class EngineLogReceiver;

class Debug {
public:
	static void SetLogReceiver(EngineLogReceiver* cb);

	static void Log(const std::string& message);
	static void LogWarning(const std::string& message);
	static void LogError(const std::string& message);

	static std::string Now();
	static void Break(const std::string& expression, const char* file, int line);
	static void Break(const std::string& expression, const std::string& message, const char* file, int line);

	static void EnableMemoryLeakCheck();

	static void StartSample(const std::string& text);
	static void EndSample();

	static void AssertGLImpl(const char* file, int line);

private:
	Debug();

	static void BreakMessage(const std::string& message);

private:
	static int length_;
	static std::stack<std::string> samples_;
	static EngineLogReceiver* logReceiver_;
};

#define Verify(expression) \
	(void)((!!(expression)) || (Debug::Break(#expression,  __FILE__, __LINE__), 0))

#define VerifyX(expression, message) \
	(void)((!!(expression)) || (Debug::Break(#expression, message,  __FILE__, __LINE__), 0))

#if _DEBUG
#define Assert(expression)	Verify(expression)
#define AssertX(expression, message) VerifyX(expression, message)
#define AssertGL() Debug::AssertGLImpl(__FILE__, __LINE__)
#else
#define Assert(expression) (void)0
#define AssertX(expression, message) (void)0
#define AssertGL() (void)0
#endif
