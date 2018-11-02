#pragma once
#include "enginedefines.h"
#include "tools/singleton.h"
#include "frameeventlistener.h"

class SUEDE_API Engine : private Singleton2<Engine> {
	friend class Singleton<Engine>;
	SUEDE_DECLARE_IMPLEMENTATION(Engine)

public:
	static bool Startup(uint width, uint height);
	static void Shutdown();

	static void Update();

	static void AddFrameEventListener(FrameEventListener* listener);
	static void RemoveFrameEventListener(FrameEventListener* listener);

private:
	Engine();
};
