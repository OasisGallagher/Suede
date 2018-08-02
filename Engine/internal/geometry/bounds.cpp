#include "bounds.h"
#include "tools/math2.h"
#include "debug/debug.h"

const Bounds Bounds::empty;

Bounds::Bounds() : Bounds(glm::vec3(0), glm::vec3(0)) {
}

Bounds::Bounds(const glm::vec3& center, const glm::vec3& size) :center(center), size(size) {
}

bool Bounds::IsEmpty() const {
	return size == glm::vec3(0);
}

void Bounds::Clear() {
	center = size = glm::vec3(0);
}

void Bounds::SetMinMax(const glm::vec3& min, const glm::vec3& max) {
	center = (min + max) / 2.f;
	size = max - min;
}

void Bounds::Expand(const glm::vec3& amount) {
	size *= amount;
}

void Bounds::Translate(const glm::vec3& amount) {
	center += amount;
}

glm::vec3 Bounds::GetMin() const {
	return center - size / 2.f;
}

glm::vec3 Bounds::GetMax() const {
	return center + size / 2.f;
}

glm::vec3 Bounds::GetSize() const {
	return size;
}

void Bounds::Encapsulate(const Bounds& other) {
	if (IsEmpty()) {
		center = other.center;
		size = other.size;
	}
	else if(!other.IsEmpty()) {
		SetMinMax(glm::min(GetMin(), other.GetMin()), glm::max(GetMax(), other.GetMax()));
	}
}

void Bounds::Encapsulate(const glm::vec3& point) {
	if (IsEmpty()) {
		center = point;
		size = glm::vec3(std::numeric_limits<float>::min());
	}
	else {
		SetMinMax(glm::min(GetMin(), point), glm::max(GetMax(), point));
	}
}
