#pragma once
#include "defines.h"

#include "math/vector3.h"
#include "math/vector4.h"

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
	/**
	 * Distance from the origin to the plane.
	 */
	float GetDistance() const { return d_; }

	/**
	* Normal vector of the plane.
	*/
	const Vector3& GetNormal() const { return normal_; }

	/**
	 * Returns a signed distance from plane to point.
	 * The value returned is positive if the point is on the side of the plane into which 
	 * the plane's normal is facing, and negative otherwise.
	 */
	float GetDistanceToPoint(const Vector3& p) const { return Vector3::Dot(normal_, p) + d_; }

private:
	void Normalize();

private:
	float d_;
	Vector3 normal_;
};
