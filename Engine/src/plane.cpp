#include "plane.h"

Plane::Plane(const glm::vec4& abcd) : Plane(glm::vec3(abcd), abcd.w) {
	Normalize();
}

Plane::Plane(const glm::vec3 points[3]) 
	: Plane(glm::cross(points[1] - points[0], points[2] - points[1]), -glm::dot(normal_, points[0])) {
}

void Plane::Normalize() {
	float mag = glm::length(normal_);
	normal_ /= mag;
	d_ /= mag;
}
