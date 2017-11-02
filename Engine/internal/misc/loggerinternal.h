#include "logger.h"

class LoggerInternal : public ILogger {
public:
	virtual void Log(const std::string & message);
	virtual void LogWarning(const std::string & message);
	virtual void LogError(const std::string & message);
};
