#pragma once
#include "../types.h"

struct SUEDE_API Vector3 {
	float x, y, z;

	Vector3() :x(0), y(0), z(0) {}
	explicit Vector3(float scalar) :x(scalar), y(scalar), z(scalar) {}
	Vector3(float x, float y, float z) :x(x), y(y), z(z) {}
	Vector3(const Vector3& other) : x(other.x), y(other.y), z(other.z) {}

	float GetMagnitude() const;
	float GetSqrMagnitude() const { return (x *x + y * y + z * z); }

	Vector3 GetNormalized() const;

	bool operator != (const Vector3& other) const;
	bool operator == (const Vector3& other) const;

	float& operator[](int i) { return *(&x + i); }
	const float& operator[](int i) const { return *(&x + i); }

	Vector3 operator+(const Vector3& other) const { return Vector3(x + other.x, y + other.y, z + other.z); }
	Vector3& operator+=(const Vector3& other);

	Vector3 operator-(const Vector3& other) const { return Vector3(x - other.x, y - other.y, z - other.z); }
	Vector3& operator-=(const Vector3& other);

	Vector3 operator*(const Vector3& other) const { return Vector3(x * other.x, y * other.y, z * other.z); }
	Vector3& operator*=(const Vector3& other);

	Vector3 operator*(float scalar) const { return Vector3(x * scalar, y * scalar, z * scalar); }
	Vector3& operator*=(float scalar);

	friend Vector3 operator*(float a, const Vector3& b);

	Vector3 operator/(const Vector3& other) const { return Vector3(x / other.x, y / other.y, z / other.z); }
	Vector3& operator/=(const Vector3& other);

	Vector3 operator/(float scalar) const { return Vector3(x / scalar, y / scalar, z / scalar); }
	Vector3& operator/=(float scalar);

	Vector3 operator-() const { return Vector3(-x, -y, -z); }

	static Vector3 Min(const Vector3& a, const Vector3& b);
	static Vector3 Max(const Vector3& a, const Vector3& b);

	static Vector3 Lerp(const Vector3& x, const Vector3& y, float t) { return x * (1.f - t) + (y * t); }

	static float Dot(const Vector3& a, const Vector3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
	static Vector3 Cross(const Vector3& a, const Vector3& b) { return Vector3(a.y * b.z - b.y * a.z, a.z * b.x - b.z * a.x, a.x * b.y - b.x * a.y); }

	static void Normalize(Vector3& a);
};

inline Vector3 operator*(float a, const Vector3& b) {
	return Vector3(a * b.x, a * b.y, a * b.z);
}

struct SUEDE_API IVector3 {
	int x, y, z;
	IVector3() :x(0), y(0), z(0) {}
	IVector3(int scalar) :x(scalar), y(scalar), z(scalar) {}
	IVector3(int x, int y, int z) :x(x), y(y), z(z) {}
};
