#pragma once

#include "component.h"

// A "rigid body" is what it sounds like ¨C an item of fixed mass, size, and other physical properties. 
// It¡¯s the base individual item in physics simulations.
class SUEDE_API IRigidbody : public IComponent {
	SUEDE_DECLARE_COMPONENT()
	SUEDE_DEFINE_METATABLE_NAME(Rigidbody)
	SUEDE_DECLARE_IMPLEMENTATION(Rigidbody)

public:
	IRigidbody();

public:
	void SetMass(float value);
	float GetMass() const;

	void SetVelocity(const glm::vec3& value);
	glm::vec3 GetVelocity() const;
};

SUEDE_DEFINE_OBJECT_POINTER(Rigidbody)
