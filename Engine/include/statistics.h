#pragma once
#include "enginedefines.h"
#include "tools/singleton.h"

class SUEDE_API Statistics : private Singleton2<Statistics> {
	friend class Singleton<Statistics>;
	SUEDE_DECLARE_IMPLEMENTATION(Statistics)

public:
	static void AddTriangles(uint n);
	static void AddDrawcalls(uint n);

	static uint GetTriangles();
	static uint GetDrawcalls();
	static float GetFrameRate();

	static void SetScriptElapsed(double value);
	static double GetScripeElapsed();

	static void SetCullingElapsed(double value);
	static double GetCullingElapsed();

	static void SetRenderingElapsed(double value);
	static double GetRenderingElapsed();

private:
	Statistics();
};
