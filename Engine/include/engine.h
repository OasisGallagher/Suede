#pragma once
#include "enginedefines.h"
#include "tools/singleton.h"
#include "frameeventlistener.h"

class SUEDE_API Engine : public Singleton2<Engine> {
	friend class Singleton<Engine>;
	SUEDE_DECLARE_IMPLEMENTATION(Engine)

public:
	bool Startup(uint width, uint height);
	void Shutdown();

	void Update();

	void AddFrameEventListener(FrameEventListener* listener);
	void RemoveFrameEventListener(FrameEventListener* listener);

private:
	Engine();
};
