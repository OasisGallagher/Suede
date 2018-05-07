#pragma once

class SUEDE_API Statistics {
public:
	static void AddTriangles(uint n);
	static void AddDrawcalls(uint n);

	static uint GetTriangles();
	static uint GetDrawcalls();
	static float GetFrameRate();

private:
	friend class Engine;
	static void Update();

private:
	Statistics();
};
