#include "plane.h"

Plane::Plane() : Plane(Vector4(0, 0, 1, 0)) {
}

Plane::Plane(const Vector4& abcd) : Plane(Vector3(abcd.x, abcd.y, abcd.z), abcd.w) {
	Normalize();
}

Plane::Plane(const Vector3 points[3]) 
	: Plane(Vector3::Cross(points[1] - points[0], points[2] - points[1]), -Vector3::Dot(normal_, points[0])) {
}

Plane::Plane(const Vector3& normal, float d) : normal_(normal), d_(d) {
}

float Plane::GetDistance() const {
	return d_;
}

Vector3 Plane::GetNormal() const {
	return normal_;
}

void Plane::Normalize() {
	float mag = normal_.GetMagnitude();
	normal_ /= mag;
	d_ /= mag;
}
