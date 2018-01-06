#include <ctime>
#include <windows.h>

#include "os.h"

IPromptCallback* promptCallback;

void OS::SetPromptCallback(IPromptCallback* value) {
	promptCallback = value;
}

void OS::EnableMemoryLeakCheck() {
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
}

time_t OS::GetFileLastWriteTime(const char* fileName) {
	HANDLE hFile = CreateFileA(fileName, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		return 0;
	}

	FILETIME ftWrite;
	SYSTEMTIME utcTime, localTime;

	GetFileTime(hFile, NULL, NULL, &ftWrite);
	FileTimeToSystemTime(&ftWrite, &utcTime);

	SystemTimeToTzSpecificLocalTime(NULL, &utcTime, &localTime);

	tm temptm = {
		localTime.wSecond,
		localTime.wMinute,
		localTime.wHour,
		localTime.wDay,
		localTime.wMonth - 1,
		localTime.wYear - 1900,
		localTime.wDayOfWeek,
	};

	return mktime(&temptm);
}

bool OS::Prompt(const char* message) {
	if (promptCallback != nullptr) {
		return promptCallback->OnPrompt(message);
	}

	return false;
}
