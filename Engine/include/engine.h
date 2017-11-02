#pragma once
#include "world.h"
#include "defines.h"

#include "timef.h"
#include "screen.h"
#include "logger.h"
#include "graphics.h"

enum {
	LogLevelDebug,
	LogLevelWarning,
	LogLevelError,
	LogLevelFatal,
};

class EngineLogReceiver {
public:
	virtual void OnEngineLogMessage(int level, const char* message) = 0;
};

class ENGINE_EXPORT Engine {
public:
	Engine();

public:
	static Engine* get();

public:
	bool initialize();
	void release();

	void setLogReceiver(EngineLogReceiver* receiver);

	void update();
	void resize(int w, int h);

	World world();

	Time time();
	Logger logger();
	Screen screen();
	Graphics graphics();
};
