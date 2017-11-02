#pragma once

class Vector4 {
public:
	Vector4();
	Vector4(float x);
	Vector4(float x, float y);
	Vector4(float x, float y, float z);
	Vector4(float x, float y, float z, float w);

public:
	static Vector4 one;
	static Vector4 zero;

private:
	float x_, y_, z_, w_;
};
