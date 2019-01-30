#pragma once

#include <intrin.h>
#pragma intrinsic(_BitScanForward)

#include "../types.h"
#include "../debug/debug.h"

// use full relative path here.
// as some projects may not set glm as 'Additional Include Directories'.
#include "../3rdparty/glm-0.9.7.1/include/glm/glm.hpp"
#include "../3rdparty/glm-0.9.7.1/include/glm/gtc/quaternion.hpp"

template <class CountofType, size_t sizeOfArray>
inline char(*__countof_helper(CountofType(&_Array)[sizeOfArray]))[sizeOfArray] {
	return nullptr;
}

#define SUEDE_COUNTOF(array) (sizeof(*__countof_helper(array)) + 0)

#ifdef SUEDE_EXPORT_MATH
#define SUEDE_MATH_API __declspec(dllexport)
#else
#define SUEDE_MATH_API __declspec(dllimport)
#endif

class SUEDE_MATH_API Math {
public:
	static const float Pi;
	static const float Pi2;
	static const float PiOver2;
	static const float Epsilon;

public:
	static int MakeDword(int low, int high);
	static int Loword(int dword);
	static int Highword(int dword);

	static float Sin(float x);
	static float Cos(float x);
	static float Asin(float x);
	static float ACos(float x);
	static float Tan(float x);
	static float ATan(float x);
	static float ATan2(float y, float x);
	static float Abs(float x);
	static float Ceil(float x);
	static float Floor(float x);
	static float Sqrt(float x);

	static float Sign(float x);

	template <class T>
	static T Degrees(const T& radians);

	template <class T>
	static T Radians(const T& degrees);

	static bool IsPowerOfTwo(uint x);
	static uint NextPowerOfTwo(uint x);
	static uint RoundUpToPowerOfTwo(uint x, uint target);

	/**
	 * @brief convert polar to euclidean coordinates.
	 * @return dir.x(theta, [0, PI]), dir.y(phi, [-PI, PI]).
	 */
	static glm::vec2 EuclideanToPolar(const glm::vec3& dir);

	/**
	 * @brief convert polar to euclidean coordinates.
	 * @return dir.x(theta, [-PI, PI]), dir.y(phi, [-PI, PI]).
	 */
	static glm::vec2 EuclideanToPolar(const glm::vec3& dir, const glm::vec3& up);

	static glm::vec3 PolarToEuclidean(const glm::vec2& polar);

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

	static bool Approximately(float lhs, float rhs);
	static bool Approximately(const glm::vec2& lhs, const glm::vec2& rhs);
	static bool Approximately(const glm::vec3& lhs, const glm::vec3& rhs);
	static bool Approximately(const glm::vec4& lhs, const glm::vec4& rhs);
	static bool Approximately(const glm::quat& lhs, const glm::quat& rhs);
	static bool Approximately(const glm::mat2& lhs, const glm::mat2& rhs);
	static bool Approximately(const glm::mat3& lhs, const glm::mat3& rhs);
	static bool Approximately(const glm::mat4& lhs, const glm::mat4& rhs);

private:
	Math();
};

inline int Math::Loword(int dword) {
	return dword & 0xffff;
}

inline int Math::Highword(int dword) {
	return (dword >> 16) & 0xffff;
}

inline int Math::MakeDword(int low, int high) {
	return (low & 0xffff) | ((high & 0xffff) << 16);
}

inline float Math::Sin(float x) {
	return sinf(x);
}

inline float Math::Cos(float x) {
	return cosf(x);
}

inline float Math::Asin(float x) {
	return asinf(x);
}

inline float Math::ACos(float x) {
	return acosf(x);
}

inline float Math::Tan(float x) {
	return tanf(x);
}

inline float Math::ATan(float x) {
	return atanf(x);
}

inline float Math::ATan2(float y, float x) {
	return atan2f(y, x);
}

inline float Math::Abs(float x) {
	return fabs(x);
}

inline float Math::Ceil(float x) {
	return ceilf(x);
}

inline float Math::Floor(float x) {
	return floorf(x);
}

inline float Math::Sqrt(float x) {
	return sqrtf(x);
}

inline float Math::Sign(float x) {
	return Math::Approximately(x, 0) ? 0.f : (x > 0 ? 1.f : -1.f);
}

template <class T>
inline T Math::Degrees(const T& radians) {
	return glm::degrees(radians);
}

template <class T>
inline T Math::Radians(const T& degrees) {
	return glm::radians(degrees);
}

inline bool Math::IsPowerOfTwo(uint x) {
	return (x & (x - 1)) == 0;
}

inline glm::vec2 Math::EuclideanToPolar(const glm::vec3& dir) {
	return glm::vec2(ACos(dir.y), ATan2(dir.z, dir.x));
}

inline glm::vec3 Math::PolarToEuclidean(const glm::vec2& polar) {
	return glm::vec3(
		Sin(polar.x) * Cos(polar.y),
		Cos(polar.x),
		Sin(polar.x) * Sin(polar.y)
	);
}

inline uint Math::RoundUpToPowerOfTwo(uint x, uint target) {
	--target;
	return (x + target) & (~target);
}

inline uint Math::Log2PowerOfTwo(uint x) {
	ulong index;
	_BitScanForward(&index, x);
	return index;
}

template <class T>
inline T Math::Lerp(const T& from, const T& to, float t) {
	return from + (to - from) * t;
}

template <>
inline glm::quat Math::Lerp(const glm::quat& from, const glm::quat& to, float t) {
	return glm::lerp(from, to, t);
}

inline float Math::Repeat(float t, float length) {
	return fmod(t, length);
}

template <class T>
inline T Math::Min(T x, T y) { return glm::min(x, y); }

template <class T>
inline T Math::Max(T x, T y) { return glm::max(x, y); }

template <class T>
inline T Math::Clamp(T value, T min, T max) {
	if (value < min) { value = min; }
	if (value > max) { value = max; }
	return value;
}

template <class T>
inline T Math::Clamp01(T value) {
	if (value < 0) { value = 0; }
	if (value > 1) { value = 1; }
	return value;
}

inline bool Math::Approximately(float lhs, float rhs) {
	return Abs(lhs - rhs) < Epsilon;
}

inline bool Math::Approximately(const glm::vec2& lhs, const glm::vec2& rhs) {
	return Math::Approximately(lhs.x, rhs.x) && Math::Approximately(lhs.y, rhs.y);
}

inline bool Math::Approximately(const glm::vec3& lhs, const glm::vec3& rhs) {
	return Math::Approximately(lhs.x, rhs.x) && Math::Approximately(lhs.y, rhs.y) && Math::Approximately(lhs.z, rhs.z);
}

inline bool Math::Approximately(const glm::vec4& lhs, const glm::vec4& rhs) {
	return Math::Approximately(lhs.x, rhs.x) && Math::Approximately(lhs.y, rhs.y) && Math::Approximately(lhs.z, rhs.z) && Math::Approximately(lhs.w, rhs.w);
}

inline bool Math::Approximately(const glm::quat& lhs, const glm::quat& rhs) {
	return 1.f - Abs(glm::dot(lhs, rhs)) < Epsilon;
}

inline bool Math::Approximately(const glm::mat2& lhs, const glm::mat2& rhs) {
	return Approximately(lhs[0], rhs[0]) && Approximately(lhs[1], rhs[1]);
}

inline bool Math::Approximately(const glm::mat3& lhs, const glm::mat3& rhs) {
	return Approximately(lhs[0], rhs[0]) && Approximately(lhs[1], rhs[1]) && Approximately(lhs[2], rhs[2]);
}

inline bool Math::Approximately(const glm::mat4& lhs, const glm::mat4& rhs) {
	return Approximately(lhs[0], rhs[0]) && Approximately(lhs[1], rhs[1]) && Approximately(lhs[2], rhs[2]) && Approximately(lhs[3], rhs[3]);
}
