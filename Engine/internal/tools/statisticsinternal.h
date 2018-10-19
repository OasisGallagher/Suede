#pragma once
#include "statistics.h"

#include "engine.h"

class StatisticsInternal : public FrameEventListener {
public:
	StatisticsInternal();

public:
	void AddTriangles(uint n);
	void AddDrawcalls(uint n);

	uint GetTriangles();
	uint GetDrawcalls();
	float GetFrameRate();

public:
	void OnFrameEnter();
	int GetFrameEventQueue() { return FrameEventQueueStatistics; }

	void SetCullingElapsed(double value);
	void SetRenderingElapsed(double value);

	double GetCullingElapsed();
	double GetRenderingElapsed();

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
