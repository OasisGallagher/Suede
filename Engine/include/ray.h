#pragma once
#include "defines.h"
#include "math/vector3.h"

class SUEDE_API Ray {
public:
	Ray();
	Ray(const Vector3& origin, const Vector3& direction);

	Vector3 GetPoint(float distance) const { return origin_ + distance * direction_; }

	const Vector3& GetOrigin() const { return origin_; }
	void SetOrigin(const Vector3& value) { origin_ = value; }

	const Vector3& GetDirection() const { return direction_; }
	void SetDirection(const Vector3& value) { direction_ = value.GetNormalized(); }

private:
	Vector3 origin_, direction_;
};

inline Ray::Ray() : direction_(0, 0, -1) {}
inline Ray::Ray(const Vector3& origin, const Vector3& direction) : origin_(origin), direction_(direction.GetNormalized()) {}
