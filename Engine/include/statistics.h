#pragma once
#include "tools/singleton.h"

class SUEDE_API Statistics : public Singleton2<Statistics> {
public:
	virtual void AddTriangles(uint n) = 0;
	virtual void AddDrawcalls(uint n) = 0;

	virtual uint GetTriangles() = 0;
	virtual uint GetDrawcalls() = 0;
	virtual float GetFrameRate() = 0;
};
