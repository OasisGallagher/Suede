#pragma once
#include "tools/singleton.h"

class SUEDE_API Statistics : public Singleton2<Statistics> {
public:
	virtual void AddTriangles(uint n) = 0;
	virtual void AddDrawcalls(uint n) = 0;

	virtual uint GetTriangles() = 0;
	virtual uint GetDrawcalls() = 0;
	virtual float GetFrameRate() = 0;

	virtual void SetCullingElapsed(double value) = 0;
	virtual void SetRenderingElapsed(double value) = 0;

	virtual double GetCullingElapsed() = 0;
	virtual double GetRenderingElapsed() = 0;
};
