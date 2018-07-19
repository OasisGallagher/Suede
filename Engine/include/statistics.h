#pragma once
#include "engine.h"
#include "tools/singleton.h"

class SUEDE_API Statistics : public Singleton<Statistics>, public FrameEventListener {
	friend class Singleton<Statistics>;

public:
	void AddTriangles(uint n);
	void AddDrawcalls(uint n);

	uint GetTriangles();
	uint GetDrawcalls();
	float GetFrameRate();

public:
	virtual void OnFrameEnter();
	virtual int GetFrameEventQueue() { return FrameEventQueueStatistics; }

private:
	Statistics();

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
