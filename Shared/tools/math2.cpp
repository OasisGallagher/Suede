#include "math2.h"

const float Math::Pi = 3.1415926f;
const float Math::Pi2 = 6.2831853f;
const float Math::PiOver2 = 1.5707963f;
const float Math::Epsilon = 0.000001f;

uint Math::NextPowerOfTwo(uint x) {
	x--;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	x++;
	return x;
}

glm::vec2 Math::EuclideanToPolar(const glm::vec3& dir, const glm::vec3& up) {
	glm::vec2 polar = EuclideanToPolar(dir);
	if (polar.y < 0 && glm::dot(glm::cross(up, dir), glm::vec3(Math::Sin(polar.y), 0, -Math::Cos(polar.y))) < 0) {
		polar.y += Pi;
		polar.x = -polar.x;
	}

	return polar;
}

uint Math::PopulationCount(uint x) {
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
