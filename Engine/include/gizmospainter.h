#pragma once

#include "behaviour.h"

class SUEDE_API IGizmosPainter : public IBehaviour {
	SUEDE_DECLARE_COMPONENT()

public:
	virtual void OnDrawGizmos() = 0;
};

SUEDE_DEFINE_OBJECT_POINTER(GizmosPainter)
