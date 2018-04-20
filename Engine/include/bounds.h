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

	void Clear();
	void Encapsulate(const Bounds& other);
	void SetMinMax(const glm::vec3& min, const glm::vec3& max);

	void Expand(const glm::vec3& amount);
	void Translate(const glm::vec3& amount);

	glm::vec3 GetMin() const;
	glm::vec3 GetMax() const;
	glm::vec3 GetSize() const;

	glm::vec3 size;
	glm::vec3 center;
};
