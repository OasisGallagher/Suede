#include "bounds.h"
#include "tools/math2.h"

Bounds::Bounds() : Bounds(glm::vec3(0), glm::vec3(0)) {
}

Bounds::Bounds(const glm::vec3& center, const glm::vec3& size) :center(center), size(size) {
}

bool Bounds::IsEmpty() const {
	return Math::Approximately(size.x) || Math::Approximately(size.y) || Math::Approximately(size.z);
}

void Bounds::SetMinMax(const glm::vec3& min, const glm::vec3& max) {
	center = (min + max) / 2.f;
	size = max - min;
}

void Bounds::Encapsulate(const Bounds& other) {
	const glm::vec3& min1 = GetMin(), &max1 = GetMax();
	const glm::vec3& min2 = other.GetMin(), &max2 = other.GetMax();

	glm::vec3 min = glm::vec3(glm::min(min1.x, min2.x), glm::min(min1.y, min2.y), glm::min(min1.z, min2.z));
	glm::vec3 max = glm::vec3(glm::max(max1.x, max2.x), glm::max(max1.y, max2.y), glm::max(max1.z, max2.z));

	SetMinMax(min, max);
}
