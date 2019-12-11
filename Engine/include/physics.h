#pragma once
#include "ray.h"
#include "gameobject.h"
#include "tools/singleton.h"

struct SUEDE_API RaycastHit {
	Vector3 point;
	Vector3 normal;

	GameObject* gameObject;
};

class SUEDE_API Physics : private Singleton2<Physics> {
	friend class Singleton<Physics>;
	SUEDE_DECLARE_IMPLEMENTATION(Physics)

public:
	static bool Raycast(const Ray& ray, float maxDistance, RaycastHit* hitInfo);

	static void SetGravity(const Vector3& value);
	static Vector3 GetGravity();

	static void SetDebugDrawEnabled(bool value);
	static bool GetDebugDrawEnabled();

private:
	Physics();
};
