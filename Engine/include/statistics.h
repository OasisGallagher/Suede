#pragma once
#include "frameeventlistener.h"

class SUEDE_API Statistics : public FrameEventListener {
public:
	virtual void OnFrameEnter();
	virtual void OnFrameLeave();

public:
	static void Initialize();

public:
	static void AddTriangles(uint n);
	static void AddDrawcalls(uint n);

	static uint GetTriangles();
	static uint GetDrawcalls();
	static float GetFrameRate();

private:
	Statistics();
};
