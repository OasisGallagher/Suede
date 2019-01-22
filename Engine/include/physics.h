#pragma once

#include <glm/glm.hpp>

#include "ray.h"
#include "gameobject.h"
#include "tools/singleton.h"

struct SUEDE_API RaycastHit {
	glm::vec3 point;
	glm::vec3 normal;

	GameObject gameObject;
};

class SUEDE_API Physics : private singleton2<Physics> {
	friend class singleton<Physics>;
	SUEDE_DECLARE_IMPLEMENTATION(Physics)

public:
	static bool Raycast(const Ray& ray, float maxDistance, RaycastHit* hitInfo);

	static void SetGravity(const glm::vec3& value);
	static glm::vec3 GetGravity();

	static void SetDebugDrawEnabled(bool value);
	static bool GetDebugDrawEnabled();

private:
	Physics();
};
