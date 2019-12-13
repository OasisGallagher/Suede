#pragma once
#include "enginedefines.h"

#include "tools/event.h"
#include "tools/singleton.h"

class SUEDE_API Engine : private Singleton2<Engine> {
	friend class Singleton<Engine>;
	SUEDE_DECLARE_IMPLEMENTATION(Engine)

public:
	static bool Startup(uint width, uint height);
	static void Shutdown();

	static void Update();

	static sorted_event<> frameEnter;
	static sorted_event<> frameLeave;

private:
	Engine();
};
