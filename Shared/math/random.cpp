#include "random.h"
#include <random>

int Random::IntRange(int min, int max) {
	static std::default_random_engine engine;
	std::uniform_int_distribution<int> distribution(min, max);
	return distribution(engine);
}

float Random::FloatRange(float min, float max) {
	static std::default_random_engine engine;
	std::uniform_real_distribution<float> distribution(min, max);
	return distribution(engine);
}

Vector3 Random::InsideSphere(float radius) {
	// https://stackoverflow.com/questions/5408276/sampling-uniformly-distributed-random-points-inside-a-spherical-volume
	float phi = FloatRange(0.f, 2 * 3.1415926f);
	float theta = acosf(FloatRange(-1.f, 1.f));
	float u = FloatRange(0.f, 1.f);

	radius *= powf(u, 1 / 3.f);
	return radius * Vector3(sinf(theta) * cosf(phi), sinf(theta) * sinf(phi), cosf(theta));
}
