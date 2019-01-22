#pragma once
#include <glm/glm.hpp>
#include "enginedefines.h"

/** 
 * @brief: Ax + By + Cz + d = 0 
 */
class SUEDE_API Plane {
public:
	Plane();
	Plane(const glm::vec4& abcd);
	Plane(const glm::vec3 points[3]);
	Plane(const glm::vec3& normal, float d);

public:
	void Normalize();
	float GetDistance() const;
	glm::vec3 GetNormal() const;

private:
	glm::vec4 abcd_;
};
