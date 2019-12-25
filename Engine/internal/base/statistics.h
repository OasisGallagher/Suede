#pragma once
#include "statistics.h"

#include "world.h"

class Statistics {
public:
	Statistics();
	~Statistics();

public:
	void AddTriangles(uint n);
	void AddDrawcalls(uint n);

public:
	void SetScriptElapsed(double value);
	void SetCullingElapsed(double value);
	void SetRenderingElapsed(double value);

	const FrameStatistics& GetFrameStatistics();

private:
	void OnFrameEnter();

private:
	float frameRate_;
	float timeCounter_;
	uint frameCounter_;

	FrameStatistics stats_[2];
};
