#pragma once
#include <glm/glm.hpp>

class Vector2 {
public:
	Vector2();
	Vector2(float x);
	Vector2(float x, float y);

public:
	static Vector2 one;
	static Vector2 zero;

private:
	float x_, y_;
};
