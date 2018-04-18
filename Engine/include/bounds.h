#pragma once
#include "types.h"
#include <glm/glm.hpp>

class Bounds {
public:
	Bounds();
	Bounds(const glm::vec3& min, const glm::vec3& max);

	void Encapsulate(const Bounds& other);
	void Create(const glm::vec3& min, const glm::vec3& max);

	const glm::vec3& GetMin() const;
	const glm::vec3& GetMax() const;

	uint GetPointCount() const { return 8; }
	const glm::vec3* GetPoints() const { return points; }

private:
	glm::vec3 points[8];
};
