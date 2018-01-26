#pragma once
#include "../shareddefines.h"

class PromptCallback {
public:
	virtual bool OnPrompt(const char* message) = 0;
};

class SHARED_API OS {
public:
	static void SetPromptCallback(PromptCallback* value);
	static time_t GetFileLastWriteTime(const char* fileName);
	static bool Prompt(const char* message);
private:
	OS();
};
