#pragma once

#include "frameevent.h"
#include "enginedefines.h"

class SUEDE_API Engine {
public:
	static bool Initialize();
	static void Release();

	static void Update();
	static void Resize(int w, int h);

	static void AddFrameEventListener(FrameEventListener* listener);
	static void RemoveFrameEventListener(FrameEventListener* listener);

private:
	static void SetDefaultGLStates();
};
