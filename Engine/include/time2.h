#pragma once
#include "enginedefines.h"
#include "tools/singleton.h"

class SUEDE_API Time : public Singleton<Time> {
public:
	void Update();

public:
	/**
	 * @brief the time in seconds it took to complete the last frame.
	 */
	float GetDeltaTime();
	float GetRealTimeSinceStartup();

	uint GetFrameCount();

private:
	double deltaTime_;

	uint frameCount_;
	uint64 lastFrameTimeStamp_;
};
