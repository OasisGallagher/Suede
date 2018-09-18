#pragma once
#include "types.h"

struct SUEDE_API Color {
	Color();
	Color(float r, float g, float b, float a);

	bool operator!= (const Color other) const;
	bool operator== (const Color other) const;

	Color operator* (float amount) const;
	Color& operator*= (float amount);

	Color operator/ (float amount) const;
	Color& operator/= (float amount);

	Color operator+ (const Color& other) const;
	Color& operator+= (const Color& other);

	Color operator- (const Color& other) const;
	Color& operator-= (const Color& other);

	Color operator* (const Color& other) const;
	Color& operator*= (const Color& other);

	Color operator/ (const Color& other) const;
	Color& operator/= (const Color& other);

	float r, g, b, a;

	static Color red;
	static Color green;
	static Color blue;
	static Color black;
	static Color white;
};

inline Color operator* (float amount, const Color& color) {
	return color * amount;
}
