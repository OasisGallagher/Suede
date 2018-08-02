#pragma once
#include "statistics.h"

#include "engine.h"

class StatisticsInternal : public Statistics, public FrameEventListener {
public:
	StatisticsInternal();

public:
	virtual void AddTriangles(uint n);
	virtual void AddDrawcalls(uint n);

	virtual uint GetTriangles();
	virtual uint GetDrawcalls();
	virtual float GetFrameRate();

public:
	virtual void OnFrameEnter();
	virtual int GetFrameEventQueue() { return FrameEventQueueStatistics; }

private:
	float frameRate_;
	float timeCounter_;
	uint frameCounter_;

	struct FrameStats {
		uint ndrawcalls;
		uint nvertices;
		uint ntriangles;
	} stats_[2];
};