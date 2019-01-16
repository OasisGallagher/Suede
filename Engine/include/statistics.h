#pragma once
#include "enginedefines.h"
#include "tools/singleton.h"

class SUEDE_API Statistics : private singleton2<Statistics> {
	friend class singleton<Statistics>;
	SUEDE_DECLARE_IMPLEMENTATION(Statistics)

public:
	static void AddTriangles(uint n);
	static void AddDrawcalls(uint n);

	static uint GetTriangles();
	static uint GetDrawcalls();
	static float GetFrameRate();

	static void SetScriptElapsed(double value);
	static double GetScriptElapsed();

	static void SetPhysicsElapsed(double value);
	static double GetPhysicsElapsed();

	static void SetCullingElapsed(double value);
	static double GetCullingElapsed();

	static void SetRenderingElapsed(double value);
	static double GetRenderingElapsed();

private:
	Statistics();
};
