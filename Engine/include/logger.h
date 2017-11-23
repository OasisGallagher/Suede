#pragma once
#include <string>
#include "defines.h"

class SUEDE_API ILogger {
public:
	virtual void Log(const std::string& message) = 0;
	virtual void LogWarning(const std::string& message) = 0;
	virtual void LogError(const std::string& message) = 0;
};

SUEDE_DEFINE_OBJECT_POINTER(Logger);
