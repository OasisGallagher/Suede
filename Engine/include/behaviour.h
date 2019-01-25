#pragma once
#include "component.h"

class SUEDE_API IBehaviour : public IComponent {
	SUEDE_DECLARE_COMPONENT()

public:
	IBehaviour();
};

SUEDE_DEFINE_OBJECT_POINTER(Behaviour)
