#pragma once
#include "../types.h"

struct SUEDE_MATH_API Vector2 {
	float x, y;

	Vector2() :x(0), y(0) {}
	explicit Vector2(float scalar) :x(scalar), y(scalar) {}
	Vector2(float x, float y) :x(x), y(y) {}
	Vector2(const Vector2& other) : x(other.x), y(other.y) {}

	float GetMagnitude() const;
	float GetSqrMagnitude() const { return (x * x + y * y); }

	Vector2 GetNormalized() const;

	float& operator[](int i) { return *(&x + i); }
	const float& operator[](int i) const { return *(&x + i); }

	bool operator != (const Vector2& other) const;
	bool operator == (const Vector2& other) const;

	Vector2 operator+(const Vector2& other) const { return Vector2(x + other.x, y + other.y); }
	Vector2& operator+=(const Vector2& other);

	Vector2 operator-(const Vector2& other) const { return Vector2(x - other.x, y - other.y); }
	Vector2& operator-=(const Vector2& other);

	Vector2 operator*(const Vector2& other) const { return Vector2(x * other.x, y * other.y); }
	Vector2& operator*=(const Vector2& other);

	Vector2 operator*(float scalar) const { return Vector2(x * scalar, y * scalar); }
	Vector2& operator*=(float scalar);

	Vector2 operator/(const Vector2& other) const { return Vector2(x / other.x, y / other.y); }
	Vector2& operator/=(const Vector2& other);

	Vector2 operator-() const { return Vector2(-x, -y); }

	static Vector2 one;
	static Vector2 zero;

	static Vector2 Min(const Vector2& a, const Vector2& b);
	static Vector2 Max(const Vector2& a, const Vector2& b);

	static float Dot(const Vector2& a, const Vector2& b) { return a.x * b.x + a.y * b.y; }
	static float Cross(const Vector2& a, const Vector2& b) { return a.x * b.y - a.y * b.x; }

	static void Normalize(Vector2& a);
	static float Distance(const Vector2& a, const Vector2& b) { return (a - b).GetMagnitude(); }
	static float SqrDistance(const Vector2& a, const Vector2& b) { return (a - b).GetSqrMagnitude(); }
};

struct SUEDE_API IVector2 {
	int x, y;
	IVector2() :x(0), y(0) {}
	IVector2(int scalar) :x(scalar), y(scalar) {}
	IVector2(int x, int y) :x(x), y(y) {}

	bool operator != (const IVector2& other) const { return x != other.x || y != other.y; }
	bool operator == (const IVector2& other) const { return x == other.x && y == other.y; }
};
