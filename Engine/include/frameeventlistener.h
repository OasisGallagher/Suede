#pragma once
#include "enginedefines.h"

enum {
	FrameEventQueueTime = -1000,
	FrameEventQueueProfiler,
	FrameEventQueueStatistics,

	FrameEventQueueUser = 0,

	FrameEventQueueMax = 1000,
	FrameEventQueueGizmos,
	FrameEventQueueInputs,
};

class SUEDE_API FrameEventListener {
public:
	virtual void OnFrameEnter() {}
	virtual void OnFrameLeave() {}
	virtual int GetFrameEventQueue() { return FrameEventQueueUser; }
};
