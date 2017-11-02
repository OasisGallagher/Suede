#pragma once
#include <memory>
#include <string>
#include "defines.h"

class ENGINE_EXPORT ILogger {
public:
	virtual void Log(const std::string& message) = 0;
	virtual void LogWarning(const std::string& message) = 0;
	virtual void LogError(const std::string& message) = 0;
};

typedef std::shared_ptr<ILogger> Logger;
