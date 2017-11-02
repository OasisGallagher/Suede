#pragma once

class Vector3 {
public:
	Vector3();
	Vector3(float x);
	Vector3(float x, float y, float z);

public:
	static Vector3 one;
	static Vector3 zero;

	static Vector3 up;
	static Vector3 right;
	static Vector3 forward;

private:
	float x_, y_, z_;
};
