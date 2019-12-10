#pragma once
#include "math/vector3.h"
#include "math/vector4.h"
#include "enginedefines.h"

/** 
 * @brief: Ax + By + Cz + d = 0 
 */
class SUEDE_API Plane {
public:
	Plane();
	Plane(const Vector4& abcd);
	Plane(const Vector3 points[3]);
	Plane(const Vector3& normal, float d);

public:
	float GetDistance() const;
	Vector3 GetNormal() const;

private:
	void Normalize();

private:
	float d_;
	Vector3 normal_;
};
