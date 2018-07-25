#pragma once
#include "engine.h"
#include "enginedefines.h"
#include "tools/singleton.h"

class SUEDE_API Time : public Singleton<Time>, public FrameEventListener {
	friend class Singleton<Time>;

public:
	/**
	 * @brief the time in seconds at the beginning of this frame.
	 */
	float GetTime() { return time_; }

	/**
	 * @brief the time in seconds it took to complete the last frame.
	 */
	float GetDeltaTime() { return deltaTime_; }

	/**
	 * @brief the real time in seconds since the engine started.
	 */
	float GetRealTimeSinceStartup();

	/**
	 * @brief the total number of frames that have passed.
	 */
	uint GetFrameCount();

public:
	virtual void OnFrameEnter();
	virtual int GetFrameEventQueue() { return FrameEventQueueTime; }

private:
	Time();

private:
	float time_;
	float deltaTime_;

	uint frameCount_;
	uint64 lastFrameTimeStamp_;
};
