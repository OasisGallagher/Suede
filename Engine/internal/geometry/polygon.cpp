#include "plane.h"
#include "polygon.h"
#include "debug/debug.h"
#include "memory/memory.h"
#include "geometryutility.h"

Polygon::Polygon() : points(nullptr), npoints(1) {
}

Polygon::Polygon(const glm::vec3* p, uint n) {
	points = MEMORY_NEW_ARRAY(glm::vec3, n);
	if (p != nullptr) {
		memcpy(points, p, sizeof(glm::vec3) * n);
	}

	npoints = n;
}

Polygon::Polygon(const Polygon& other) : Polygon(other.points, other.npoints) {
}

Polygon::~Polygon() {
	MEMORY_DELETE_ARRAY(points);
}

Polygon& Polygon::operator=(const Polygon& other) {
	MEMORY_DELETE_ARRAY(points);
	points = MEMORY_NEW_ARRAY(glm::vec3, other.npoints);
	memcpy(points, other.points, sizeof(glm::vec3) * other.npoints);
	return *this;
}

glm::vec3& Polygon::operator[](uint index) {
	SUEDE_VERIFY_INDEX(index, npoints, points[0]);
	return points[index];
}

const glm::vec3& Polygon::operator[](uint index) const {
	SUEDE_VERIFY_INDEX(index, npoints, points[0]);
	return points[index];
}

Triangle::Triangle() : Polygon(nullptr, 3) {
}

Triangle::Triangle(const glm::vec3* p) : Polygon(p, 3) {
}
