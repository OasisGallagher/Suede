#pragma once
#include <glm/glm.hpp>
#include "enginedefines.h"

class SUEDE_API Rect {
public:
	Rect() {}
	Rect(const glm::vec2& p, const glm::vec2& size);
	Rect(float x, float y, float width, float height);

public:
	bool operator == (const Rect& other) const;
	bool operator != (const Rect& other) const;

public:
	glm::vec2 GetLeftTop() const;
	glm::vec2 GetLeftBottom() const;
	glm::vec2 GetRightTop() const;
	glm::vec2 GetRightBottom() const;

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
	glm::vec4 data_;
};
