#include "vector3.h"

#include "mathf.h"

float Vector3::GetMagnitude() const {
	return sqrtf(x *x + y * y + z * z);
}

Vector3 Vector3::GetNormalized() const {
	float len = GetMagnitude();
	return Vector3(x / len, y / len, z / len);
}

void Vector3::Normalize(Vector3& a) {
	float len = a.GetMagnitude();
	a.x /= len; a.y /= len; a.z /= len;
}

bool Vector3::operator != (const Vector3& other) const {
	return !Mathf::Approximately(x, other.x)
		|| !Mathf::Approximately(y, other.y)
		|| !Mathf::Approximately(z, other.z);
}

bool Vector3::operator == (const Vector3& other) const {
	return Mathf::Approximately(x, other.x)
		&& Mathf::Approximately(y, other.y)
		&& Mathf::Approximately(z, other.z);
}

Vector3& Vector3::operator+=(const Vector3& other) {
	x += other.x; y += other.y; z += other.z;
	return *this;
}

Vector3& Vector3::operator-=(const Vector3& other) {
	x -= other.x; y -= other.y; z -= other.z;
	return *this;
}

Vector3& Vector3::operator*=(const Vector3& other) {
	x *= other.x; y *= other.y; z *= other.z;
	return *this;
}

Vector3& Vector3::operator*=(float scalar) {
	x *= scalar; y *= scalar; z *= scalar;
	return *this;
}

Vector3& Vector3::operator/=(const Vector3& other) {
	x /= other.x; y /= other.y; z /= other.z;
	return *this;
}

Vector3& Vector3::operator/=(float scalar) {
	x /= scalar; y /= scalar; z /= scalar;
	return *this;
}

Vector3 Vector3::Min(const Vector3& a, const Vector3& b) {
	return Vector3(Mathf::Min(a.x, b.x), Mathf::Min(a.y, b.y), Mathf::Min(a.z, b.z));
}

Vector3 Vector3::Max(const Vector3& a, const Vector3& b) {
	return Vector3(Mathf::Max(a.x, b.x), Mathf::Max(a.y, b.y), Mathf::Max(a.z, b.z));
}
