#pragma once

#include "enginedefines.h"
#include "frameeventlistener.h"

class GraphicsCanvas;
class Engine {
public:
	static bool Initialize();
	static void Release();

	static void Update();

	static void AddFrameEventListener(FrameEventListener* listener);
	static void RemoveFrameEventListener(FrameEventListener* listener);
};
