#pragma once
#include "tools/event.h"
#include "enginedefines.h"

class SUEDE_API Engine {
public:
	static bool Startup(uint width, uint height);
	static void Shutdown();

	static void Update();

	static sorted_event<> frameEnter;
	static sorted_event<> frameLeave;
};
