#pragma once
#include "math/mathf.h"

struct SUEDE_API Color {
	Color();
	Color(float r, float g, float b, float a = 1);

	float GetLuminance() const;
	void Set(float r, float g, float b, float a = 1);

	bool operator!= (const Color other) const;
	bool operator== (const Color other) const;

	Color operator+ (const Color& other) const;
	Color& operator+= (const Color& other);

	Color operator- (const Color& other) const;
	Color& operator-= (const Color& other);

	Color operator* (float amount) const;
	Color& operator*= (float amount);

	Color operator* (const Color& other) const;
	Color& operator*= (const Color& other);

	Color operator/ (float amount) const;
	Color& operator/= (float amount);

	Color operator/ (const Color& other) const;
	Color& operator/= (const Color& other);

	float r, g, b, a;

	static Color red;
	static Color green;
	static Color blue;
	static Color black;
	static Color white;
	static Color yellow;
};

inline Color::Color() :r(0), g(0), b(0), a(0) {
}

inline Color::Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {
}

inline void Color::Set(float r, float g, float b, float a) {
	this->r = r, this->g = g, this->b = b, this->a = a;
}

inline float Color::GetLuminance() const {
	return 0.299f * r + 0.587f * g + 0.114f * b;
}

inline bool Color::operator!= (const Color other) const {
	return !Mathf::Approximately(r, other.r) || !Mathf::Approximately(g, other.g)
		|| !Mathf::Approximately(b, other.b) || !Mathf::Approximately(a, other.a);
}

inline bool Color::operator== (const Color other) const {
	return Mathf::Approximately(r, other.r) && Mathf::Approximately(g, other.g)
		&& Mathf::Approximately(b, other.b) && Mathf::Approximately(a, other.a);
}

inline Color Color::operator* (float amount) const {
	return Color(r * amount, g * amount, b * amount, a * amount);
}

inline Color& Color::operator*= (float amount) {
	r *= amount; g *= amount; b *= amount; a *= amount;
	return *this;
}

inline Color Color::operator/ (float amount) const {
	return Color(r / amount, g / amount, b / amount, a / amount);
}

inline Color& Color::operator/= (float amount) {
	r /= amount; g /= amount; b /= amount; a /= amount;
	return *this;
}

inline Color Color::operator+ (const Color& other) const {
	return Color(r + other.r, g + other.g, b + other.b, a + other.a);
}

inline Color& Color::operator+= (const Color & other) {
	r += other.r; g += other.g; b += other.b; a += other.a;
	return *this;
}

inline Color Color::operator- (const Color & other) const {
	return Color(r - other.r, g - other.g, b - other.b, a - other.a);
}

inline Color& Color::operator-= (const Color& other) {
	r -= other.r; g -= other.g; b -= other.b; a -= other.a;
	return *this;
}

inline Color Color::operator* (const Color& other) const {
	return Color(r * other.r, g * other.g, b * other.b, a * other.a);
}

inline Color& Color::operator*= (const Color& other) {
	r *= other.r; g *= other.g; b *= other.b; a *= other.a;
	return *this;
}

inline Color Color::operator/ (const Color& other) const {
	return Color(r / other.r, g / other.g, b / other.b, a / other.a);
}

inline Color & Color::operator/= (const Color & other) {
	r /= other.r; g /= other.g; b /= other.b; a /= other.a;
	return *this;
}

inline Color operator* (float amount, const Color& color) {
	return color * amount;
}
