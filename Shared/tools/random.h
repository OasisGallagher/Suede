#pragma once
#include <random>
#include "../3rdparty/glm-0.9.7.1/include/glm/glm.hpp"

class Random {
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
	static glm::vec3 InsideSphere(float radius);

private:
	Random();
};

inline int Random::IntRange(int min, int max) {
	static std::default_random_engine engine;
	std::uniform_int_distribution<int> distribution(min, max);
	return distribution(engine);
}

inline float Random::FloatRange(float min, float max) {
	static std::default_random_engine engine;
	std::uniform_real_distribution<float> distribution(min, max);
	return distribution(engine);
}

inline glm::vec3 Random::InsideSphere(float radius) {
	// https://stackoverflow.com/questions/5408276/sampling-uniformly-distributed-random-points-inside-a-spherical-volume
	float phi = FloatRange(0.f, 2 * 3.1415926f);
	float theta = Math::ACos(FloatRange(-1.f, 1.f));
	float u = FloatRange(0.f, 1.f);

	radius *= powf(u, 1 / 3.f);
	return radius * glm::vec3(Math::Sin(theta) * Math::Cos(phi), Math::Sin(theta) * Math::Sin(phi), Math::Cos(theta));
}
