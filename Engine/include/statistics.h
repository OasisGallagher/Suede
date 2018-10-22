#pragma once
#include "enginedefines.h"
#include "tools/singleton.h"

class SUEDE_API Statistics : public Singleton2<Statistics> {
	friend class Singleton<Statistics>;
	SUEDE_DECLARE_IMPLEMENTATION(Statistics)

public:
	void AddTriangles(uint n);
	void AddDrawcalls(uint n);

	uint GetTriangles();
	uint GetDrawcalls();
	float GetFrameRate();

	void SetCullingElapsed(double value);
	void SetRenderingElapsed(double value);

	double GetCullingElapsed();
	double GetRenderingElapsed();

private:
	Statistics();
};
