#pragma once
#include "ray.h"
#include "subsystem.h"
#include "gameobject.h"

struct SUEDE_API RaycastHit {
	Vector3 point;
	Vector3 normal;

	GameObject* gameObject;
};

class Gizmos;
class SUEDE_API Physics : public Subsystem {
	SUEDE_DECLARE_IMPLEMENTATION(Physics)
public:
	enum {
		SystemType = SubsystemType::Physics,
	};

public:
	Physics(Gizmos* gizmos);

public:
	bool Raycast(const Ray& ray, float maxDistance, RaycastHit* hitInfo);

	void SetGravity(const Vector3& value);
	Vector3 GetGravity();

	void SetDebugDrawEnabled(bool value);
	bool GetDebugDrawEnabled();

public:
	virtual void Update(float deltaTime);
};
