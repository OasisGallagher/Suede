#pragma once
#include "types.h"
#include <glm/glm.hpp>

struct Triangle {
	Triangle(const glm::vec3* p) {
		memcpy(points, p, sizeof(glm::vec3) * 3);
	}

	glm::vec3& operator[](uint index) { return points[index]; }
	const glm::vec3& operator[](uint index) const { return points[index]; }
	
	glm::vec3 points[3];
};
