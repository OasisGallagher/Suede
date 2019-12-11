#pragma once

#include "behaviour.h"

class SUEDE_API GizmosPainter : public Behaviour {
	SUEDE_DECLARE_COMPONENT()

public:
	virtual void OnDrawGizmos() = 0;
};
