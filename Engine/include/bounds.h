#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "enginedefines.h"

/**
 * @brief axis-aligned bounding box.
 */
struct SUEDE_API Bounds {
	Bounds();
	Bounds(const glm::vec3& center, const glm::vec3& size);

	bool IsEmpty() const;
	void Encapsulate(const Bounds& other);
	void SetMinMax(const glm::vec3& min, const glm::vec3& max);

	glm::vec3 GetMin() const { return center - size / 2.f; }
	glm::vec3 GetMax() const { return center + size / 2.f; }

	glm::vec3 GetSize() const { return size; }

	glm::vec3 size;
	glm::vec3 center;
};
