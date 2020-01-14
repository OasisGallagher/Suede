#pragma once
#include "vector3.h"

class SUEDE_MATH_API Random {
public:
	/**
	 * @return a random integer between and min [inclusive] and max [inclusive].
	 */
	static int IntRange(int min, int max);

	/**
	 * @return a random float number between min [inclusive] and max [exclusive].
	 */
	static float FloatRange(float min, float max);

	/**
	 * @return a random point inside a circle with radius `radius`.
	 */
	static Vector3 InsideSphere(float radius);
};
