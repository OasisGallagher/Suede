#pragma once
#include "../types.h"
#include "../debug/debug.h"

#include "vector3.h"
#include "matrix4.h"
#include "quaternion.h"

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

	static uint MakeDword(uint low, uint high);
	static uint Loword(uint dword);
	static uint Highword(uint dword);

	static float Angle(const Vector3& a, const Vector3& b, const Vector3& normal);

	static bool IsPowerOfTwo(uint x);
	static uint NextPowerOfTwo(uint x);
	static uint RoundUpToPowerOfTwo(uint x, uint target);

	static float Lerp(float from, float to, float t);

	static float Repeat(float t, float length);
	static float PingPong(float t, float length);

	template <class T> static T Sign(T x);
	template <class T> static T Min(T x, T y);
	template <class T> static T Max(T x, T y);

	static float Clamp01(float value);
	template <class T> static T Clamp(T value, T min, T max);

	static bool Approximately(float x, float y);
	static bool Approximately(const Quaternion& x, const Quaternion& y);

	static void Orthogonalize(Vector3& t, const Vector3& n);

private:
	Mathf();
};

inline uint Mathf::Loword(uint dword) {
	return dword & 0xffff;
}

inline uint Mathf::Highword(uint dword) {
	return (dword >> 16) & 0xffff;
}

inline uint Mathf::MakeDword(uint low, uint high) {
	return (low & 0xffff) | ((high & 0xffff) << 16);
}

template <class T>
inline T Mathf::Sign(T x) {
	return Mathf::Approximately(x, 0) ? 0.f : (x > 0 ? 1.f : -1.f);
}

inline float Mathf::Angle(const Vector3& a, const Vector3& b, const Vector3& normal) {
	float r = acosf(Vector3::Dot(a, b));
	if (Vector3::Dot(normal, Vector3::Cross(a, b)) < 0) {
		r = -r;
	}

	return r;
}

inline bool Mathf::IsPowerOfTwo(uint x) {
	return (x & (x - 1)) == 0;
}

inline uint Mathf::NextPowerOfTwo(uint x) {
	x--;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	x++;
	return x;
}

inline uint Mathf::RoundUpToPowerOfTwo(uint x, uint target) {
	SUEDE_ASSERT(IsPowerOfTwo(target));

	--target;
	return (x + target) & (~target);
}

inline float Mathf::Lerp(float from, float to, float t) {
	return from + (to - from) * t;
}

inline float Mathf::Repeat(float t, float length) {
	return fmod(t, length);
}

inline float Mathf::PingPong(float t, float length) {
	float L = 2 * length;
	float T = fmod(t, L);
	if (T >= 0 && T < length) { return T; }
	return L - T;
}

template <class T>
inline T Mathf::Min(T x, T y) { return x > y ? y : x; }

template <class T>
inline T Mathf::Max(T x, T y) { return x > y ? x : y; }

template <class T>
inline T Mathf::Clamp(T value, T min, T max) {
	if (value < min) { value = min; }
	if (value > max) { value = max; }
	return value;
}

inline float Mathf::Clamp01(float value) {
	if (value < 0) { value = 0; }
	if (value > 1) { value = 1; }
	return value;
}

inline bool Mathf::Approximately(float x, float y) {
	return fabs(x - y) < epsilon;
}

inline bool Mathf::Approximately(const Quaternion& x, const Quaternion& y) {
	return 1.f - fabs(Quaternion::Dot(x, y)) < epsilon;
}

inline void Mathf::Orthogonalize(Vector3& t, const Vector3& n) {
	// Gram-Schmidt orthogonalize
	t = (t - n * Vector3::Dot(n, t)).GetNormalized();
}
