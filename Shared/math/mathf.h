#pragma once
#include <random>
#include <intrin.h>

#include "../types.h"
#include "../debug/debug.h"

#include "vector3.h"
#include "matrix4.h"
#include "quaternion.h"

#pragma intrinsic(_BitScanForward)

template <class CountofType, size_t sizeOfArray>
inline char(*__countof_helper(CountofType(&_Array)[sizeOfArray]))[sizeOfArray] {
	return nullptr;
}

#define SUEDE_COUNTOF(array) (sizeof(*__countof_helper(array)) + 0)

class SUEDE_API Mathf {
public:
	static int MakeDword(int low, int high);
	static int Loword(int dword);
	static int Highword(int dword);

	static float Pi();
	static float Epsilon();

	template <class T>
	static T Sign(T x);

	template <class T>
	static T Degrees(const T& radians) {
		return radians * 57.2957795f;
	}

	template <class T>
	static T Radians(const T& degrees) {
		return degrees * 0.0174533f;
	}

	static float Angle(const Vector3& a, const Vector3& b, const Vector3& normal);

	static bool IsPowerOfTwo(uint x);
	static uint NextPowerOfTwo(uint x);
	static uint RoundUpToPowerOfTwo(uint x, uint target);

	/**
	 * @brief log_2_POT.
	 */
	static uint Log2PowerOfTwo(uint x);

	/**
	* @brief count the number of binary "1" bits in x.
	*/
	static uint PopulationCount(uint x);

	template <class T>
	static T Lerp(const T& from, const T& to, float t);

	static float Repeat(float t, float length);
	static float PingPong(float t, float length);

	template <class T>
	static T Min(T x, T y);

	template <class T>
	static T Max(T x, T y);

	template <class T>
	static T Clamp(T value, T min, T max);

	template <class T>
	static T Clamp01(T value);

	static bool Approximately(float x, float y);
	static bool Approximately(const Quaternion& x, const Quaternion& y);

	static void Orthogonalize(Vector3& t, const Vector3& n);

private:
	Mathf();
};

inline int Mathf::Loword(int dword) {
	return dword & 0xffff;
}

inline int Mathf::Highword(int dword) {
	return (dword >> 16) & 0xffff;
}

inline int Mathf::MakeDword(int low, int high) {
	return (low & 0xffff) | ((high & 0xffff) << 16);
}

inline float Mathf::Pi() {
	return 3.1415926f;
}

inline float Mathf::Epsilon() {
	return 0.000001f;
}

template <class T>
inline T Mathf::Sign(T x) {
	return Mathf::Approximately(x, 0) ? 0 : (x > 0 ? 1 : -1);
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
	if (!IsPowerOfTwo(target)) {
		Debug::LogError("target must be power of two.");
		return x;
	}

	--target;
	return (x + target) & (~target);
}

inline uint Mathf::PopulationCount(uint x) {
	uint n;

	n = (x >> 1) & 0x77777777;
	x = x - n;
	n = (n >> 1) & 0x77777777;
	x = x - n;
	n = (n >> 1) & 0x77777777;
	x = x - n;
	x = (x + (x >> 4)) & 0x0F0F0F0F;
	x = x * 0x01010101;

	return x >> 24;
}

inline uint Mathf::Log2PowerOfTwo(uint x) {
	ulong index;
	_BitScanForward(&index, x);
	return index;
}

template <class T>
inline T Mathf::Lerp(const T& from, const T& to, float t) {
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

template <class T>
inline T Mathf::Clamp01(T value) {
	if (value < 0) { value = 0; }
	if (value > 1) { value = 1; }
	return value;
}

inline bool Mathf::Approximately(float x, float y) {
	return fabs(x - y) < Epsilon();
}

inline bool Mathf::Approximately(const Quaternion& x, const Quaternion& y) {
	return 1.f - fabs(Quaternion::Dot(x, y)) < Epsilon();
}

inline void Mathf::Orthogonalize(Vector3& t, const Vector3& n) {
	// Gram-Schmidt orthogonalize
	t = (t - n * Vector3::Dot(n, t)).GetNormalized();
}
