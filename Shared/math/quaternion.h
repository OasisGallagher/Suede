#pragma once
#include "../types.h"
#include "vector3.h"

struct Matrix4;
struct SUEDE_MATH_API Quaternion {
	float x, y, z, w;

	Quaternion() :x(0), y(0), z(0), w(1) {}
	explicit Quaternion(const Matrix4& m);
	explicit Quaternion(const Vector3& eulerAngle);
	Quaternion(float angle, const Vector3& axis);
	Quaternion(float w, float x, float y, float z) : w(w), x(x), y(y), z(z) {}

	Vector3 GetEulerAngles() const;
	Quaternion GetInversed() const;
	Quaternion GetNormalized() const;
	Quaternion GetConjugated() const { return Quaternion(w, -x, -y, -z); }

	Quaternion operator+(const Quaternion& other) const { return Quaternion(*this) += other; }
	Quaternion operator+=(const Quaternion& other);

	Quaternion operator*(const Quaternion& other) const { return Quaternion(*this) *= other; }
	Quaternion& operator*=(const Quaternion& other);

	Vector3 operator*(const Vector3& dir) const;

	Quaternion operator*(float scalar) const { return Quaternion(*this) *= scalar; }
	Quaternion operator*=(float scalar);

	bool operator ==(const Quaternion& other) const;
	bool operator !=(const Quaternion& other) const;

	static void Pow(Quaternion& q, float x);
	static float Dot(const Quaternion& a, const Quaternion& b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }
	static void Inverse(Quaternion& q) { q = q.GetInversed(); }
	static void Normalize(Quaternion& q) { q = q.GetNormalized(); }
	static void Conjugate(Quaternion& q) { q.x = -q.x; q.y = -q.y; q.z = -q.z; }

	static Quaternion Lerp(const Quaternion& a, const Quaternion& b, float t);
	static Quaternion Slerp(const Quaternion& a, const Quaternion& b, float t);
};
