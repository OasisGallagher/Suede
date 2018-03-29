#include "plane.h"
#include "polygon.h"
#include "memory/memory.h"
#include "geometryutility.h"

#define CHECK_INDEX(index)	if (index >= npoints) { Debug::LogError("index out of range."); return points[0]; } else (void)0

Polygon::Polygon(const glm::vec3 * p, uint n) {
	points = MEMORY_CREATE_ARRAY(glm::vec3, n);
	memcpy(points, p, sizeof(glm::vec3) * n);

	npoints = n;
}

Polygon::~Polygon() {
	MEMORY_RELEASE_ARRAY(points);
}

Polygon& Polygon::operator=(const Polygon& other) {
	MEMORY_RELEASE_ARRAY(points);
	points = MEMORY_CREATE_ARRAY(glm::vec3, other.npoints);
	memcpy(points, other.points, sizeof(glm::vec3) * other.npoints);
	return *this;
}

glm::vec3& Polygon::operator[](uint index) {
	CHECK_INDEX(index);
	return points[index];
}

const glm::vec3& Polygon::operator[](uint index) const {
	CHECK_INDEX(index);
	return points[index];
}
