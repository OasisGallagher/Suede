#pragma once

#include "bounds.h"
#include "component.h"

/**
 * A "rigid body" is what it sounds like ¨C an item of fixed mass, size, and other physical properties.
 * It's the base individual item in physics simulations.
 */
class SUEDE_API Rigidbody : public Component {
	SUEDE_DECLARE_COMPONENT()
	SUEDE_DEFINE_METATABLE_NAME(Rigidbody)
	SUEDE_DECLARE_IMPLEMENTATION(Rigidbody)

public:
	Rigidbody();

public:
	void ShowCollisionShape(bool value);

	void SetMass(float value);
	float GetMass() const;

	void SetVelocity(const Vector3& value);
	Vector3 GetVelocity() const;
};
