#pragma once
#include "types.h"
#include <glm/glm.hpp>

struct Polygon {
	Polygon(const glm::vec3* p, uint n);
	Polygon(const Polygon& other) : Polygon(other.points, other.npoints) {}

	virtual ~Polygon();

	Polygon& operator = (const Polygon& other);

	glm::vec3& operator[](uint index);
	const glm::vec3& operator[](uint index) const;

	uint npoints;
	glm::vec3* points;
};

struct Triangle : Polygon {
	Triangle(const glm::vec3* p) : Polygon(p, 3) {}
};
