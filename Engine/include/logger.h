#pragma once
#include <string>
#include "defines.h"

class ENGINE_EXPORT ILogger {
public:
	virtual void Log(const std::string& message) = 0;
	virtual void LogWarning(const std::string& message) = 0;
	virtual void LogError(const std::string& message) = 0;
};

DEFINE_OBJECT_PTR(Logger);
