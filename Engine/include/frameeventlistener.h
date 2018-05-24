#pragma once
#include "enginedefines.h"

class SUEDE_API FrameEventListener {
public:
	virtual void OnFrameEnter() {}
	virtual void OnFrameLeave() {}
	virtual int GetFrameEventQueue() { return 0; }
};
