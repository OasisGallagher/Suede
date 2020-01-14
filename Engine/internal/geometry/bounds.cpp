#include "bounds.h"
#include "math/mathf.h"
#include "debug/debug.h"

const Bounds Bounds::empty;

Bounds::Bounds() : Bounds(Vector3(0), Vector3(0)) {
}

Bounds::Bounds(const Vector3& center, const Vector3& size) : center(center), size(size) {
}

bool Bounds::IsEmpty() const {
	return size == Vector3(0);
}

void Bounds::Clear() {
	center = size = Vector3(0);
}

void Bounds::SetMinMax(const Vector3& min, const Vector3& max) {
	center = (min + max) / 2.f;
	size = max - min;
}

void Bounds::Expand(const Vector3& amount) {
	size *= amount;
}

Vector3 Bounds::GetMin() const {
	return center - size / 2.f;
}

Vector3 Bounds::GetMax() const {
	return center + size / 2.f;
}

Vector3 Bounds::GetSize() const {
	return size;
}

void Bounds::Encapsulate(const Bounds& other) {
	if (IsEmpty()) {
		center = other.center;
		size = other.size;
	}
	else if(!other.IsEmpty()) {
		SetMinMax(Vector3::Min(GetMin(), other.GetMin()), Vector3::Max(GetMax(), other.GetMax()));
	}
}

void Bounds::Encapsulate(const Vector3* points, int npoints) {
	SUEDE_ASSERT(points != nullptr && npoints >= 1);
	Vector3 min(std::numeric_limits<float>::max());
	Vector3 max(std::numeric_limits<float>::lowest());

	for (int i = 0; i < npoints; ++i) {
		min = Vector3::Min(min, points[i]);
		max = Vector3::Max(max, points[i]);
	}

	Encapsulate(Bounds((min + max) / 2.f, max - min));
}
