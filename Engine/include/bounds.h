#pragma once
#include <vector>

#include "defines.h"
#include "math/vector3.h"

/**
 * @brief Axis-aligned bounding box.
 */
struct SUEDE_API Bounds {
	Bounds();
	Bounds(const Vector3& center, const Vector3& size);

	bool IsEmpty() const;

	void Clear();
	void Encapsulate(const Bounds& other);
	void Encapsulate(const Vector3* points, int npoints);
	void SetMinMax(const Vector3& min, const Vector3& max);

	void Expand(const Vector3& amount);
	void Translate(const Vector3& amount);

	Vector3 GetMin() const;
	Vector3 GetMax() const;
	Vector3 GetSize() const;

	Vector3 size;
	Vector3 center;

	static const Bounds empty;
};
