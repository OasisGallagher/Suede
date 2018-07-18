#pragma once
#include "tools/singleton.h"

class SUEDE_API Statistics : public Singleton<Statistics> {
public:
	void Update();

public:
	void AddTriangles(uint n);
	void AddDrawcalls(uint n);

	uint GetTriangles();
	uint GetDrawcalls();
	float GetFrameRate();

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
