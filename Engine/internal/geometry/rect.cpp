#include "rect.h"
#include "math/mathf.h"

Rect Rect::unit(0, 0, 1, 1);

Rect::Rect(const Vector2& p, const Vector2& size) : Rect(p.x, p.y, size.x, size.y) {
}

Rect::Rect(float x, float y, float width, float height) {
	data_ = Vector4(x, y, width, height);
}

bool Rect::operator == (const Rect & other) const {
	return data_ == other.data_;
}

bool Rect::operator != (const Rect & other) const {
	return data_ != other.data_;
}

Vector2 Rect::GetLeftTop() const {
	return Vector2(data_.x, data_.y + data_.w);
}

Vector2 Rect::GetLeftBottom() const {
	return Vector2(data_.x, data_.y);
}

Vector2 Rect::GetRightTop() const {
	return Vector2(data_.x + data_.z, data_.y + data_.w);
}

Vector2 Rect::GetRightBottom() const {
	return Vector2(data_.x + data_.z, data_.y);
}

Vector2 Rect::GetSize() {
	return Vector2(data_.z, data_.w);
}

void Rect::SetWidth(float value) {
	data_.z = value;
}

float Rect::GetWidth() const {
	return data_.z;
}

void Rect::SetHeight(float value) {
	data_.w = value;
}

float Rect::GetHeight() const {
	return data_.w;
}

void Rect::SetXMin(float value) {
	data_.x = value;
}

float Rect::GetXMin() const {
	return data_.x;
}

void Rect::SetYMin(float value) {
	data_.y = value;
}

float Rect::GetYMin() const {
	return data_.y;
}

Rect Rect::NormalizedToRect(const Rect& rect, const Rect& normalized) {
	return Rect(
		rect.GetXMin() + rect.GetWidth() * normalized.GetXMin(),
		rect.GetYMin() + rect.GetHeight() * normalized.GetYMin(),
		rect.GetWidth() * Mathf::Clamp(normalized.GetWidth(), 0.f, 1 - normalized.GetXMin()),
		rect.GetHeight() * Mathf::Clamp(normalized.GetHeight(), 0.f, 1 - normalized.GetYMin())
	);
}
