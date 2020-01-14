#pragma once
#include "../types.h"

struct SUEDE_MATH_API Vector4 {
	float x, y, z, w;
	
	Vector4() :x(0), y(0), z(0), w(0) {}
	explicit Vector4(float scalar) :x(scalar), y(scalar), z(scalar), w(scalar) {}
	Vector4(float x, float y, float z, float w) :x(x), y(y), z(z), w(w) {}
	Vector4(const Vector4& other) : x(other.x), y(other.y), z(other.z), w(other.w) {}

	float GetMagnitude() const;
	float GetSqrMagnitude() const { return (x *x + y * y + z * z + w * w); }

	Vector4 GetNormalized() const;

	bool operator != (const Vector4& other) const;
	bool operator == (const Vector4& other) const;

	float& operator[](int i) { return *(&x + i); }
	const float& operator[](int i) const { return *(&x + i); }

	Vector4 operator+(const Vector4& other) const { return Vector4(x + other.x, y + other.y, z + other.z, w + other.w); }
	Vector4& operator+=(const Vector4& other);

	Vector4 operator-(const Vector4& other) const { return Vector4(x - other.x, y - other.y, z - other.z, w - other.w); }
	Vector4& operator-=(const Vector4& other);

	Vector4 operator*(const Vector4& other) const { return Vector4(x * other.x, y * other.y, z * other.z, w * other.w); }
	Vector4& operator*=(const Vector4& other);

	Vector4 operator*(float scalar) const { return Vector4(x * scalar, y * scalar, z * scalar, w *scalar); }
	Vector4& operator*=(float x);

	Vector4 operator/(const Vector4& other) const { return Vector4(x / other.x, y / other.y, z / other.z, w / other.w); }
	Vector4& operator/=(const Vector4& other);

	Vector4 operator/(float scalar) const { return Vector4(x / scalar, y / scalar, z / scalar, w / scalar); }
	Vector4& operator/=(float x);

	Vector4 operator-() const { return Vector4(-x, -y, -z, -w); }

	static Vector4 one;
	static Vector4 zero;

	static float Dot(const Vector4& a, const Vector4& b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }

	static void Normalize(Vector4& a);
	static float Distance(const Vector4& a, const Vector4& b) { return (a - b).GetMagnitude(); }
	static float SqrDistance(const Vector4& a, const Vector4& b) { return (a - b).GetSqrMagnitude(); }
};

struct SUEDE_API IVector4 {
	int x, y, z, w;
	IVector4() :x(0), y(0), z(0), w(0) {}
	IVector4(int scalar) :x(scalar), y(scalar), z(scalar), w(scalar) {}
	IVector4(int x, int y, int z, int w) :x(x), y(y), z(z), w(w) {}

	bool operator != (const IVector4& other) const { return x != other.x || y != other.y || z != other.z || w != other.w; }
	bool operator == (const IVector4& other) const { return x == other.x && y == other.y && z == other.z && w == other.w; }
};
