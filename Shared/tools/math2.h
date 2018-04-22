#pragma once
#include <intrin.h>

#include "../shareddefines.h"
#include "../glm/glm.hpp"
#include "../glm/gtc/quaternion.hpp"

#pragma intrinsic(_BitScanForward)

template<class CountofTpe, size_t sizeOfArray>
inline char(*__countof_helper(CountofTpe(&_Array)[sizeOfArray]))[sizeOfArray] {
	return nullptr;
}

#define CountOf(array) (sizeof(*__countof_helper(array)) + 0)
#define Padding(size) char __unused[size]

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
	static T Random(T min, T max);
	static glm::vec3 RandomInsideSphere(float r);

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
	--target;
	return (x + target) & (~target);
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
inline T Math::Random(T min, T max) {
	return min + (rand() % (max - min + 1));
}

template <>
inline float Math::Random(float min, float max) {
	float random = ((float)rand()) / (float)RAND_MAX;
	return (random * (max - min)) + min;
}

inline glm::vec3 Math::RandomInsideSphere(float r) {
	// https://stackoverflow.com/questions/5408276/sampling-uniformly-distributed-random-points-inside-a-spherical-volume
	float phi = Random(0.f, 2 * Math::Pi());
	float theta = acosf(Random(-1.f, 1.f));
	float u = Random(0.f, 1.f);

	r *= powf(u, 1 / 3.f);
	return r * glm::vec3(sinf(theta) * cosf(phi), sinf(theta) * sinf(phi), cosf(theta));
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
