#pragma once
#include "engine.h"
#include "enginedefines.h"
#include "tools/singleton.h"

class SUEDE_API Time : public Singleton<Time>, public FrameEventListener {
	friend class Singleton<Time>;

public:
	/**
	 * @brief the time in seconds it took to complete the last frame.
	 */
	float GetDeltaTime();
	float GetRealTimeSinceStartup();

	uint GetFrameCount();

public:
	virtual void OnFrameEnter();
	virtual int GetFrameEventQueue() { return FrameEventQueueTime; }

private:
	Time();

private:
	double deltaTime_;

	uint frameCount_;
	uint64 lastFrameTimeStamp_;
};
