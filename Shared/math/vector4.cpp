#include "vector4.h"

#include "mathf.h"

Vector4 Vector4::one(1);
Vector4 Vector4::zero(0);

float Vector4::GetMagnitude() const {
	return sqrtf(x * x + y * y + z * z + w * w);
}

Vector4 Vector4::GetNormalized() const {
	float len = GetMagnitude();
	return Vector4(x / len, y / len, z / len, w / len);
}

Vector4& Vector4::operator+=(const Vector4& other) {
	x += other.x; y += other.y; z += other.z; w += other.w;
	return *this;
}

Vector4& Vector4::operator-=(const Vector4& other) {
	x -= other.x; y -= other.y; z -= other.z; w -= other.w;
	return *this;
}

Vector4& Vector4::operator*=(const Vector4& other) {
	x *= other.x; y *= other.y; z *= other.z; w *= other.w;
	return *this;
}

Vector4& Vector4::operator*=(float scalar) {
	x *= scalar; y *= scalar; z *= scalar; w *= scalar;
	return *this;
}

Vector4& Vector4::operator/=(const Vector4& other) {
	x /= other.x; y /= other.y; z /= other.z; w /= other.w;
	return *this;
}

Vector4& Vector4::operator/=(float scalar) {
	x /= scalar; y /= scalar; z /= scalar; w /= scalar;
	return *this;
}

void Vector4::Normalize(Vector4& a) {
	float len = a.GetMagnitude();
	a.x /= len; a.y /= len; a.z /= len; a.w /= len;
}

bool Vector4::operator != (const Vector4& other) const {
	return !Mathf::Approximately(x, other.x)
		|| !Mathf::Approximately(y, other.y)
		|| !Mathf::Approximately(z, other.z)
		|| !Mathf::Approximately(w, other.w);
}

bool Vector4::operator == (const Vector4& other) const {
	return Mathf::Approximately(x, other.x)
		&& Mathf::Approximately(y, other.y)
		&& Mathf::Approximately(z, other.z)
		&& Mathf::Approximately(w, other.w);
}
