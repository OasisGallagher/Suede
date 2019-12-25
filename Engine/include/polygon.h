#pragma once
#include "defines.h"

struct SUEDE_API Polygon {
	Polygon();
	Polygon(const Polygon& other);
	Polygon(const Vector3* p, uint n);

	virtual ~Polygon();

	Polygon& operator = (const Polygon& other);

	Vector3& operator[](uint index);
	const Vector3& operator[](uint index) const;

	uint npoints;
	Vector3* points;
};

struct SUEDE_API Triangle : Polygon {
	Triangle();
	Triangle(const Vector3* p);
};
