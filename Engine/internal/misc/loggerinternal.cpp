#include "tools/debug.h"
#include "loggerinternal.h"
#include "internal/memory/memory.h"

Logger loggerInstance(Memory::Create<LoggerInternal>());

void LoggerInternal::Log(const std::string & message) {
	Debug::Log(message);
}

void LoggerInternal::LogWarning(const std::string & message) {
	Debug::LogWarning(message);
}

void LoggerInternal::LogError(const std::string & message) {
	Debug::LogError(message);
}
