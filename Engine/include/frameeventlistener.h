#pragma once
#include "enginedefines.h"

enum {
	FrameEventQueueTime = -3000,
	FrameEventQueueProfiler = -2000,
	FrameEventQueueStatistics = -1000,

	FrameEventQueueUser = 0,
};

class SUEDE_API FrameEventListener {
public:
	virtual void OnFrameEnter() {}
	virtual void OnFrameLeave() {}
	virtual int GetFrameEventQueue() { return FrameEventQueueUser; }
};
