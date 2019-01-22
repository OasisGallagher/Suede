#include "plane.h"

Plane::Plane() : Plane(glm::vec4(0, 0, 1, 0)) {
}

Plane::Plane(const glm::vec4& abcd) : Plane(glm::vec3(abcd), abcd.w) {
	Normalize();
}

Plane::Plane(const glm::vec3 points[3]) 
	: Plane(glm::cross(points[1] - points[0], points[2] - points[1]), -glm::dot(glm::vec3(abcd_.xyz), points[0])) {
}

Plane::Plane(const glm::vec3& normal, float d) : abcd_(glm::vec4(normal, d)) { }

float Plane::GetDistance() const {
	return abcd_.w;
}

glm::vec3 Plane::GetNormal() const {
	return abcd_.xyz;
}

void Plane::Normalize() {
	float mag = glm::length(glm::vec3(abcd_.xyz));
	abcd_ /= mag;
}
