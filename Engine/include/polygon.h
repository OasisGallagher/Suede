#pragma once
#include <glm/glm.hpp>
#include "enginedefines.h"

struct SUEDE_API Polygon {
	Polygon();
	Polygon(const Polygon& other);
	Polygon(const glm::vec3* p, uint n);

	virtual ~Polygon();

	Polygon& operator = (const Polygon& other);

	glm::vec3& operator[](uint index);
	const glm::vec3& operator[](uint index) const;

	uint npoints;
	glm::vec3* points;
};

struct Triangle : Polygon {
	Triangle();
	Triangle(const glm::vec3* p);
};
