#pragma once
#include <cmath>

#include "../types.h"
#include "../debug/debug.h"

template <class CountofType, size_t sizeOfArray>
inline char(*__countof_helper(CountofType(&_Array)[sizeOfArray]))[sizeOfArray] {
	return nullptr;
}

#define SUEDE_COUNTOF(array) (sizeof(*__countof_helper(array)) + 0)

class SUEDE_MATH_API Mathf {
public:
	static const float pi;
	static const float epsilon;

	static const float rad2Deg;
	static const float deg2Rad;

	static uint MakeDword(uint low, uint high) { return (low & 0xffff) | ((high & 0xffff) << 16); }
	static uint Loword(uint dword) { return dword & 0xffff; }
	static uint Highword(uint dword) { return (dword >> 16) & 0xffff; }

	static bool IsPowerOfTwo(uint x) { return (x & (x - 1)) == 0; }
	static uint NextPowerOfTwo(uint x);
	static uint RoundUpToPowerOfTwo(uint x, uint target);

	static float Lerp(float from, float to, float t) { return from + (to - from) * t; }

	static float Repeat(float t, float length) { return fmod(t, length); }
	static float PingPong(float t, float length);

	template <class T> static T Min(T x, T y) { return x > y ? y : x; }
	template <class T> static T Max(T x, T y) { { return x > y ? x : y; } }
	template <class T> static T Sign(T x) { return Mathf::Approximately(x, 0) ? 0.f : (x > 0 ? 1.f : -1.f); }

	static float Clamp01(float value);
	template <class T> static T Clamp(T value, T min, T max);

	static bool Approximately(float x, float y) { return fabs(x - y) < epsilon; }
};

template <class T>
inline T Mathf::Clamp(T value, T min, T max) {
	if (value < min) { value = min; }
	if (value > max) { value = max; }
	return value;
}
