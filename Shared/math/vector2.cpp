#include "vector2.h"

#include "mathf.h"

float Vector2::GetMagnitude() const {
	return sqrtf(x *x + y * y);
}

Vector2 Vector2::GetNormalized() const {
	float len = GetMagnitude();
	return Vector2(x / len, y / len);
}

Vector2& Vector2::operator+=(const Vector2& other) {
	x += other.x; y += other.y;
	return *this;
}

Vector2& Vector2::operator/=(const Vector2& other) {
	x /= other.x; y /= other.y;
	return *this;
}

Vector2& Vector2::operator-=(const Vector2& other) {
	x -= other.x; y -= other.y;
	return *this;
}

Vector2& Vector2::operator*=(const Vector2& other) {
	x *= other.x; y *= other.y;
	return *this;
}

bool Vector2::operator != (const Vector2& other) const {
	return !Mathf::Approximately(x, other.x)
		|| !Mathf::Approximately(y, other.y);
}

bool Vector2::operator == (const Vector2& other) const {
	return Mathf::Approximately(x, other.x)
		&& Mathf::Approximately(y, other.y);
}

Vector2 Vector2::Min(const Vector2& a, const Vector2& b) {
	return Vector2(Mathf::Min(a.x, b.x), Mathf::Min(a.y, b.y));
}

Vector2 Vector2::Max(const Vector2& a, const Vector2& b) {
	return Vector2(Mathf::Max(a.x, b.x), Mathf::Max(a.y, b.y));
}

void Vector2::Normalize(Vector2 & a) {
	float len = a.GetMagnitude();
	a.x /= len; a.y /= len;
}
