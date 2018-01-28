#pragma once
#include "../shareddefines.h"
#include "../glm/glm.hpp"
#include "../glm/gtc/quaternion.hpp"

template<class CountofType, size_t sizeOfArray>
inline char(*__countof_helper(CountofType(&_Array)[sizeOfArray]))[sizeOfArray] {
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

	static float Degrees(float radians);
	static float Radians(float degrees);

	static glm::vec3 Degrees(const glm::vec3& radians);
	static glm::vec3 Radians(const glm::vec3& degrees);

	static unsigned NextPowerOfTwo(unsigned x);
	static unsigned RoundUpToPowerOfTwo(unsigned x, unsigned target);

	template <class Ty>
	static Ty Lerp(const Ty& from, const Ty& to, float t);

	static float Repeat(float t, float length);
	static float PingPong(float t, float length);

	template <class Ty>
	static Ty Random(Ty min, Ty max);
	static glm::vec3 RandomInsideSphere(float r);

	template <class Ty>
	static Ty Min(Ty x, Ty y);

	template <class Ty>
	static Ty Max(Ty x, Ty y);

	template <class Ty>
	static Ty Clamp(Ty value, Ty min, Ty max);

	template <class Ty>
	static Ty Clamp01(Ty value);

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

inline float Math::Degrees(float radians) {
	return radians * 57.29578f;
}

inline float Math::Radians(float degrees) {
	return degrees * 0.0174532924f;
}

inline glm::vec3 Math::Degrees(const glm::vec3 & radians) {
	return glm::vec3(Degrees(radians.x), Degrees(radians.y), Degrees(radians.z));
}

inline glm::vec3 Math::Radians(const glm::vec3 & degrees) {
	return glm::vec3(Radians(degrees.x), Radians(degrees.y), Radians(degrees.z));
}

inline unsigned Math::NextPowerOfTwo(unsigned x) {
	x--;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	x++;
	return x;
}

inline unsigned Math::RoundUpToPowerOfTwo(unsigned x, unsigned target) {
	--target;
	return (x + target) & (~target);
}

template <class Ty>
inline Ty Math::Lerp(const Ty& from, const Ty& to, float t) {
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

template <class Ty>
inline Ty Math::Random(Ty min, Ty max) {
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

template <class Ty>
inline Ty Math::Min(Ty x, Ty y) { return x > y ? y : x; }

template <class Ty>
inline Ty Math::Max(Ty x, Ty y) { return x > y ? x : y; }

template <class Ty>
inline Ty Math::Clamp(Ty value, Ty min, Ty max) {
	if (value < min) { value = min; }
	if (value > max) { value = max; }
	return value;
}

template <class Ty>
inline Ty Math::Clamp01(Ty value) {
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
