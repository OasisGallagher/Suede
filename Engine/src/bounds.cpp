#include "bounds.h"

Bounds::Bounds() {
	Create(glm::vec3(std::numeric_limits<float>::max()), glm::vec3(std::numeric_limits<float>::min()));
}

Bounds::Bounds(const glm::vec3& min, const glm::vec3& max) {
	Create(min, max);
}

void Bounds::Encapsulate(const Bounds& other) {
	const glm::vec3& min1 = GetMin(), &max1 = GetMax();
	const glm::vec3& min2 = other.GetMin(), &max2 = other.GetMax();

	glm::vec3 min = glm::vec3(glm::min(min1.x, min2.x), glm::min(min1.y, min2.y), glm::min(min1.z, min2.z));
	glm::vec3 max = glm::vec3(glm::max(max1.x, max2.x), glm::max(max1.y, max2.y), glm::max(max1.z, max2.z));

	Create(min, max);
}

void Bounds::Create(const glm::vec3& min, const glm::vec3& max) {
	points[0] = glm::vec3(min.x, min.y, min.z);
	points[1] = glm::vec3(min.x, min.y, max.z);
	points[2] = glm::vec3(max.x, min.y, max.z);
	points[3] = glm::vec3(max.x, min.y, min.z);

	points[4] = glm::vec3(min.x, max.y, min.z);
	points[5] = glm::vec3(min.x, max.y, max.z);
	points[6] = glm::vec3(max.x, max.y, max.z);
	points[7] = glm::vec3(max.x, max.y, min.z);
}

const glm::vec3 & Bounds::GetMin() const {
	return points[0];
}

const glm::vec3 & Bounds::GetMax() const {
	return points[6];
}
