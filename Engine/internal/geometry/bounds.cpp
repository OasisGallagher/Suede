#include "bounds.h"
#include "math/mathf.h"
#include "debug/debug.h"

const Bounds Bounds::empty;

Bounds::Bounds() : Bounds(Vector3(0), Vector3(0)) {
}

Bounds::Bounds(const Vector3& center, const Vector3& size) :center(center), size(size) {
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

void Bounds::Translate(const Vector3& amount) {
	center += amount;
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

void Bounds::Encapsulate(const Vector3& point) {
	if (IsEmpty()) {
		center = point;
		size = Vector3(std::numeric_limits<float>::min());
	}
	else {
		SetMinMax(Vector3::Min(GetMin(), point), Vector3::Max(GetMax(), point));
	}
}
