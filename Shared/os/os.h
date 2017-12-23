#pragma once
#include <crtdbg.h>
#include "../shareddefines.h"

#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)

class IPromptCallback {
public:
	virtual bool OnPrompt(const char* message) = 0;
};

class SHARED_API OS {
public:
	static void SetPromptCallback(IPromptCallback* value);

	static void EnableMemoryLeakCheck();
	static time_t GetFileLastWriteTime(const char* fileName);

	static bool Prompt(const char* message);
private:
	OS();
};
