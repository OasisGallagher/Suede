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

	virtual void SetCullingElapsed(double value);
	virtual void SetRenderingElapsed(double value);

	virtual double GetCullingElapsed();
	virtual double GetRenderingElapsed();

private:
	float frameRate_;
	float timeCounter_;
	uint frameCounter_;

	struct FrameStats {
		void Reset();

		uint ndrawcalls;
		uint nvertices;
		uint ntriangles;

		double cullingElapsed;
		double renderingElapsed;
	} stats_[2];
};
