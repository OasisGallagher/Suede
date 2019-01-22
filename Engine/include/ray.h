#pragma once
#include <glm/glm.hpp>
#include "enginedefines.h"

class SUEDE_API Ray {
public:
	Ray();
	Ray(const glm::vec3& origin, const glm::vec3& direction);

	glm::vec3 GetPoint(float distance) const { return origin_ + distance * direction_; }

	const glm::vec3& GetOrigin() const { return origin_; }
	void SetOrigin(const glm::vec3& value) { origin_ = value; }

	const glm::vec3& GetDirection() const { return direction_; }
	void SetDirection(const glm::vec3& value) { direction_ = glm::normalize(value); }

private:
	glm::vec3 origin_, direction_;
};

inline Ray::Ray() : direction_(0, 0, -1) {}
inline Ray::Ray(const glm::vec3& origin, const glm::vec3& direction) : origin_(origin), direction_(glm::normalize(direction)) {}
