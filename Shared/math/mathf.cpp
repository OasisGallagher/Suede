#include "mathf.h"

const float Mathf::pi = 3.1415926f;
const float Mathf::epsilon = 0.000001f;

const float Mathf::rad2Deg = 57.2957795f;
const float Mathf::deg2Rad = 0.0174533f;

uint Mathf::NextPowerOfTwo(uint x) {
	x--;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	x++;
	return x;
}

uint Mathf::RoundUpToPowerOfTwo(uint x, uint target) {
	SUEDE_ASSERT(IsPowerOfTwo(target));

	--target;
	return (x + target) & (~target);
}

float Mathf::PingPong(float t, float length) {
	float len2 = 2 * length;
	float remainder = fmod(t, len2);
	if (remainder >= 0 && remainder < length) { return remainder; }
	return len2 - remainder;
}

float Mathf::Clamp01(float value) {
	if (value < 0) { value = 0; }
	if (value > 1) { value = 1; }
	return value;
}
