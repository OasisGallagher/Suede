#include "quaternion.h"

#include "mathf.h"

Quaternion::Quaternion(const Vector3& eulerAngle) {
	Vector3 c(eulerAngle * 0.5f);
	Vector3 s(eulerAngle * 0.5f);

	c.x = cosf(c.x); c.y = cosf(c.y); c.z = cosf(c.z);
	s.x = sinf(s.x); s.y = sinf(s.y); s.z = sinf(s.z);

	w = c.x * c.y * c.z + s.x * s.y * s.z;
	x = s.x * c.y * c.z - c.x * s.y * s.z;
	y = c.x * s.y * c.z + s.x * c.y * s.z;
	z = c.x * c.y * s.z - s.x * s.y * c.z;
}

Quaternion::Quaternion(const Matrix4& m) {
	float fourXSquaredMinus1 = m.m[0][0] - m.m[1][1] - m.m[2][2];
	float fourYSquaredMinus1 = m.m[1][1] - m.m[0][0] - m.m[2][2];
	float fourZSquaredMinus1 = m.m[2][2] - m.m[0][0] - m.m[1][1];
	float fourWSquaredMinus1 = m.m[0][0] + m.m[1][1] + m.m[2][2];

	int biggestIndex = 0;
	float fourBiggestSquaredMinus1 = fourWSquaredMinus1;
	if (fourXSquaredMinus1 > fourBiggestSquaredMinus1) {
		fourBiggestSquaredMinus1 = fourXSquaredMinus1;
		biggestIndex = 1;
	}
	if (fourYSquaredMinus1 > fourBiggestSquaredMinus1) {
		fourBiggestSquaredMinus1 = fourYSquaredMinus1;
		biggestIndex = 2;
	}
	if (fourZSquaredMinus1 > fourBiggestSquaredMinus1) {
		fourBiggestSquaredMinus1 = fourZSquaredMinus1;
		biggestIndex = 3;
	}

	float biggestVal = sqrt(fourBiggestSquaredMinus1 + float(1)) * float(0.5);
	float mult = static_cast<float>(0.25) / biggestVal;

	Quaternion Result;
	switch (biggestIndex) {
	case 0:
		Result.w = biggestVal;
		Result.x = (m.m[1][2] - m.m[2][1]) * mult;
		Result.y = (m.m[2][0] - m.m[0][2]) * mult;
		Result.z = (m.m[0][1] - m.m[1][0]) * mult;
		break;
	case 1:
		Result.w = (m.m[1][2] - m.m[2][1]) * mult;
		Result.x = biggestVal;
		Result.y = (m.m[0][1] + m.m[1][0]) * mult;
		Result.z = (m.m[2][0] + m.m[0][2]) * mult;
		break;
	case 2:
		Result.w = (m.m[2][0] - m.m[0][2]) * mult;
		Result.x = (m.m[0][1] + m.m[1][0]) * mult;
		Result.y = biggestVal;
		Result.z = (m.m[1][2] + m.m[2][1]) * mult;
		break;
	case 3:
		Result.w = (m.m[0][1] - m.m[1][0]) * mult;
		Result.x = (m.m[2][0] + m.m[0][2]) * mult;
		Result.y = (m.m[1][2] + m.m[2][1]) * mult;
		Result.z = biggestVal;
		break;
	}
}

Quaternion::Quaternion(float angle, const Vector3& axis) {
	float const a(angle);
	float const s = sinf(a * 0.5f);

	w = cosf(a * 0.5f);
	x = axis.x * s;
	y = axis.y * s;
	z = axis.z * s;
}

Vector3 Quaternion::GetEulerAngles() const {
	return Vector3(
		atan2f(2 * (y * z + w * x), w * w - x * x - y * y + z * z),
		asinf(-2 * (x * z - w * y)),
		atan2f(2 * (x * y + w * z), w * w + x * x - y * y - z * z)
	);
}

void Quaternion::Pow(Quaternion& q, float x) {
	//Raising to the power of 0 should yield 1
	//Needed to prevent a division by 0 error later on
	if (x > -Mathf::Epsilon() && x < Mathf::Epsilon()) {
		q.w = 1;
		q.x = q.y = q.z = 0;
	}
	else {
		//To deal with non-unit quaternions
		float magnitude = sqrtf(q.x * q.x + q.y * q.y + q.z * q.z + q.w *q.w);

		//Equivalent to raising a real number to a power
		//Needed to prevent a division by 0 error later on
		if (fabs(q.w / magnitude) > 1.f - Mathf::Epsilon() && fabs(q.w / magnitude) < 1.f + Mathf::Epsilon()) {
			q.w = powf(q.w, x);
			q.x = q.y = q.z = 0;
		}
		else {
			float Angle = acosf(q.w / magnitude);
			float NewAngle = Angle * x;
			float Div = sinf(NewAngle) / sinf(Angle);
			float Mag = powf(magnitude, x - 1);

			q.w = cosf(NewAngle) * magnitude * Mag;
			q.x *= Div * Mag;
			q.y *= Div * Mag;
			q.z *= Div * Mag;
		}
	}
}

Quaternion Quaternion::GetInversed() const {
	float d = Dot(*this, *this);
	return Quaternion(w / d, -x / d, -y / d, -z / d);
}

Quaternion Quaternion::GetNormalized() const {
	float len = sqrtf(Dot(*this, *this));
	if (len <= 0.f) {// Problem
		return Quaternion(1, 0, 0, 0);
	}

	float oneOverLen = 1.f / len;
	return Quaternion(w * oneOverLen, x * oneOverLen, y * oneOverLen, z * oneOverLen);
}

Quaternion Quaternion::operator+=(const Quaternion& other) {
	w += other.w;  x += other.x; y += other.y; z += other.z;
	return *this;
}

Quaternion& Quaternion::operator*=(const Quaternion& other) {
	Quaternion const p(*this);
	Quaternion const q(other);

	w = p.w * q.w - p.x * q.x - p.y * q.y - p.z * q.z;
	x = p.w * q.x + p.x * q.w + p.y * q.z - p.z * q.y;
	y = p.w * q.y + p.y * q.w + p.z * q.x - p.x * q.z;
	z = p.w * q.z + p.z * q.w + p.x * q.y - p.y * q.x;

	return *this;
}

Quaternion Quaternion::operator*=(float scalar) {
	w *= scalar; x *= scalar; y *= scalar; z *= scalar;
	return *this;
}

Vector3 Quaternion::operator*(const Vector3& dir) const {
	Vector3 const QuatVector(x, y, z);
	Vector3 const uv(Vector3::Cross(QuatVector, dir));
	Vector3 const uuv(Vector3::Cross(QuatVector, uv));

	return dir + ((uv * w) + uuv) * 2.f;
}

Quaternion Quaternion::Lerp(const Quaternion& a, const Quaternion& b, float t) {
	return a * (1.f - t) + (b * t);
}

Quaternion Quaternion::Slerp(const Quaternion& a, const Quaternion& b, float t) {
	Quaternion c = b;

	float cosTheta = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;

	// If cosTheta < 0, the interpolation will take the long way around the sphere. 
	// To fix this, one quat must be negated.
	if (cosTheta < 0.f) {
		c = Quaternion(-b.x, -b.y, -b.z, -b.w);
		cosTheta = -cosTheta;
	}

	// Perform a linear interpolation when cosTheta is close to 1 to avoid side effect of sin(angle) becoming a zero denominator
	if (cosTheta > 1.f - Mathf::Epsilon()) {
		// Linear interpolation
		return Quaternion(
			Mathf::Lerp(a.w, c.w, t),
			Mathf::Lerp(a.x, c.x, t),
			Mathf::Lerp(a.y, c.y, t),
			Mathf::Lerp(a.z, c.z, t)
		);
	}

	// Essential Mathematics, page 467
	float angle = acosf(cosTheta);
	float s0 = sinf((1.f - t) * angle);
	float s1 = sinf(t * angle);
	float sa = sinf(angle);
	return Quaternion(
		(s0 *a.x + s1 * c.x) / sa,
		(s0 *a.y + s1 * c.y) / sa,
		(s0 *a.z + s1 * c.z) / sa,
		(s0 *a.w + s1 * c.w) / sa
	);
}
