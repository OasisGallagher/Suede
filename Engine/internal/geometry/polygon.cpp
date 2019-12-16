#include "plane.h"
#include "polygon.h"
#include "debug/debug.h"
#include "memory/refptr.h"
#include "geometryutility.h"

Polygon::Polygon() : points(nullptr), npoints(1) {
}

Polygon::Polygon(const Vector3* p, uint n) {
	points = new Vector3[n];
	if (p != nullptr) {
		memcpy(points, p, sizeof(Vector3) * n);
	}

	npoints = n;
}

Polygon::Polygon(const Polygon& other) : Polygon(other.points, other.npoints) {
}

Polygon::~Polygon() {
	delete[] points;
}

Polygon& Polygon::operator=(const Polygon& other) {
	if (&other != this) {
		delete[] points;
		points = new Vector3[other.npoints];
		memcpy(points, other.points, sizeof(Vector3) * other.npoints);
	}

	return *this;
}

Vector3& Polygon::operator[](uint index) {
	SUEDE_VERIFY_INDEX(index, npoints, points[0]);
	return points[index];
}

const Vector3& Polygon::operator[](uint index) const {
	SUEDE_VERIFY_INDEX(index, npoints, points[0]);
	return points[index];
}

Triangle::Triangle() : Polygon(nullptr, 3) {
}

Triangle::Triangle(const Vector3* p) : Polygon(p, 3) {
}
