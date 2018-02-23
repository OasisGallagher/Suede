#include "plane.h"

Plane::Plane(const glm::vec4& abcd) : Plane(glm::vec3(abcd), abcd.w) {
	Normalize();
}

void Plane::Normalize() {
	float mag = glm::length(normal_);
	normal_ /= mag;
	d_ /= mag;
}
