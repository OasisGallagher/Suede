#pragma once
#include <random>
#include <intrin.h>

#include "../types.h"
#include "../debug/debug.h"

// use full relative path here.
// as some projects may not set glm as 'Additional Include Directories'.
#include "../3rdparty/glm-0.9.7.1/include/glm/glm.hpp"
#include "../3rdparty/glm-0.9.7.1/include/glm/gtc/quaternion.hpp"
#include "../3rdparty/glm-0.9.7.1/include/glm/gtc/matrix_transform.hpp"

#pragma intrinsic(_BitScanForward)

template <class CountofType, size_t sizeOfArray>
inline char(*__countof_helper(CountofType(&_Array)[sizeOfArray]))[sizeOfArray] {
	return nullptr;
}

#define SUEDE_COUNTOF(array) (sizeof(*__countof_helper(array)) + 0)

class Math {
public:
	static int MakeDword(int low, int high);
	static int Loword(int dword);
	static int Highword(int dword);

	static float Pi();

	template <class T>
	static T Degrees(const T& radians);

	template <class T>
	static T Radians(const T& degrees);

	static float Angle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& normal);

	static glm::ivec4 IntColor(const glm::vec4& color);
	static glm::vec4 NormalizedColor(const glm::ivec4& color);

	static glm::ivec3 IntColor(const glm::vec3& color);
	static glm::vec3 NormalizedColor(const glm::ivec3& color);

	static bool IsPowerOfTwo(uint x);
	static uint NextPowerOfTwo(uint x);
	static uint RoundUpToPowerOfTwo(uint x, uint target);

	static glm::mat4 TRS(const glm::vec3& t, const glm::quat& r, const glm::vec3& s);

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

	static bool Approximately(float x, float y = 0.f);

	static float Luminance(const glm::vec3& color);

	static void Orthogonalize(glm::vec3& t, const glm::vec3& n);

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

inline float Math::Pi() {
	return 3.1415926f;
}

inline float Math::Angle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& normal) {
	float r = acosf(glm::dot(a, b));
	if (glm::dot(normal, glm::cross(a, b)) < 0) {
		r = -r;
	}

	return r;
}

inline glm::ivec4 Math::IntColor(const glm::vec4& color) {
	return glm::ivec4(
		Clamp(int(color.x * 255), 0, 255),
		Clamp(int(color.y * 255), 0, 255),
		Clamp(int(color.z * 255), 0, 255),
		Clamp(int(color.w * 255), 0, 255)
	);
}

inline glm::vec4 Math::NormalizedColor(const glm::ivec4& color) {
	return glm::vec4(
		Clamp01(color.x / 255.f),
		Clamp01(color.y / 255.f),
		Clamp01(color.z / 255.f),
		Clamp01(color.w / 255.f)
	);
}

inline glm::ivec3 Math::IntColor(const glm::vec3& color) {
	return glm::ivec3(
		Clamp(int(color.x * 255), 0, 255),
		Clamp(int(color.y * 255), 0, 255),
		Clamp(int(color.z * 255), 0, 255)
	);
}

inline glm::vec3 Math::NormalizedColor(const glm::ivec3& color) {
	return glm::vec3(
		Clamp01(color.x / 255.f),
		Clamp01(color.y / 255.f),
		Clamp01(color.z / 255.f)
	);
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

inline uint Math::NextPowerOfTwo(uint x) {
	x--;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	x++;
	return x;
}

inline uint Math::RoundUpToPowerOfTwo(uint x, uint target) {
	if (!IsPowerOfTwo(target)) {
		Debug::LogError("target must be power of two.");
		return x;
	}

	--target;
	return (x + target) & (~target);
}

inline glm::mat4 Math::TRS(const glm::vec3 & t, const glm::quat & r, const glm::vec3 & s) {
	return glm::translate(glm::mat4(1), t) * glm::scale(glm::mat4_cast(r), s);
}

inline uint Math::PopulationCount(uint x) {
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

inline float Math::PingPong(float t, float length) {
	float L = 2 * length;
	float T = fmod(t, L);
	if (T >= 0 && T < length) { return T; }
	return L - T;
}

template <class T>
inline T Math::Min(T x, T y) { return x > y ? y : x; }

template <class T>
inline T Math::Max(T x, T y) { return x > y ? x : y; }

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

inline bool Math::Approximately(float x, float y) {
	const float E = 0.000001f;
	return fabs(x - y) < E;
}

inline float Math::Luminance(const glm::vec3& color) {
	return 0.299f * color.r + 0.587f * color.g + 0.114f * color.b;
}

inline void Math::Orthogonalize(glm::vec3& t, const glm::vec3& n) {
	// Gram-Schmidt orthogonalize
	t = glm::normalize(t - n * glm::dot(n, t));
}
