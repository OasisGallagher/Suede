#include "plane.h"

Plane::Plane() : Plane(glm::vec4(0, 0, 1, 0)) {
}

Plane::Plane(const glm::vec4& abcd) : Plane(glm::vec3(abcd), abcd.w) {
	Normalize();
}

Plane::Plane(const glm::vec3 points[3]) 
	: Plane(glm::cross(points[1] - points[0], points[2] - points[1]), -glm::dot(normal_, points[0])) {
}


Plane::Plane(const glm::vec3& normal, float d) : normal_(normal), d_(d) {
}

float Plane::GetDistance() const {
	return d_;
}

glm::vec3 Plane::GetNormal() const {
	return normal_;
}

void Plane::Normalize() {
	float mag = glm::length(normal_);
	normal_ /= mag;
	d_ /= mag;
}
