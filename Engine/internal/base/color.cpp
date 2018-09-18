#include "color.h"
#include "tools/math2.h"

Color Color::red(1, 0, 0, 1);
Color Color::green(0, 1, 0, 1);
Color Color::blue(0, 0, 1, 1);
Color Color::black(0, 0, 0, 1);
Color Color::white(1, 1, 1, 1);

Color::Color() :r(0), g(0), b(0), a(0) {
}

Color::Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {
}

bool Color::operator!= (const Color other) const {
	return !Math::Approximately(r, other.r) || !Math::Approximately(g, other.g)
		|| !Math::Approximately(b, other.b) || !Math::Approximately(a, other.a);
}

bool Color::operator== (const Color other) const {
	return Math::Approximately(r, other.r) && Math::Approximately(g, other.g) 
		&& Math::Approximately(b, other.b) && Math::Approximately(a, other.a);
}

Color Color::operator* (float amount) const {
	return Color(r * amount, g * amount, b * amount, a * amount);
}

Color & Color::operator*= (float amount) {
	r *= amount; g *= amount; b *= amount; a *= amount;
	return *this;
}

Color Color::operator/ (float amount) const {
	return Color(r / amount, g / amount, b / amount, a / amount);
}

Color & Color::operator/= (float amount) {
	r /= amount; g /= amount; b /= amount; a /= amount;
	return *this;
}

Color Color::operator+ (const Color& other) const {
	return Color(r + other.r, g + other.g, b + other.b, a + other.a);
}

Color & Color::operator+= (const Color & other) {
	r += other.r; g += other.g; b += other.b; a += other.a;
	return *this;
}

Color Color::operator- (const Color & other) const {
	return Color(r - other.r, g - other.g, b - other.b, a - other.a);
}

Color & Color::operator-= (const Color& other) {
	r -= other.r; g -= other.g; b -= other.b; a -= other.a;
	return *this;
}

Color Color::operator* (const Color& other) const {
	return Color(r * other.r, g * other.g, b * other.b, a * other.a);
}

Color& Color::operator*= (const Color& other) {
	r *= other.r; g *= other.g; b *= other.b; a *= other.a;
	return *this;
}

Color Color::operator/ (const Color& other) const {
	return Color(r / other.r, g / other.g, b / other.b, a / other.a);
}

Color & Color::operator/= (const Color & other) {
	r /= other.r; g /= other.g; b /= other.b; a /= other.a;
	return *this;
}
