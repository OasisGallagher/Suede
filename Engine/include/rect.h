#pragma once

#include "defines.h"
#include "math/vector2.h"
#include "math/vector4.h"

class SUEDE_API Rect {
public:
	Rect() {}
	Rect(const Vector2& p, const Vector2& size);
	Rect(float x, float y, float width, float height);

public:
	bool operator == (const Rect& other) const;
	bool operator != (const Rect& other) const;

public:
	Vector2 GetLeftTop() const;
	Vector2 GetLeftBottom() const;
	Vector2 GetRightTop() const;
	Vector2 GetRightBottom() const;

public:
	void SetWidth(float value);
	float GetWidth() const;

	void SetHeight(float value);
	float GetHeight() const;

	void SetXMin(float value);
	float GetXMin() const;

	void SetYMin(float value);
	float GetYMin() const;

public:
	static Rect NormalizedToRect(const Rect& rect, const Rect& normalized);

private:
	Vector4 data_;
};
