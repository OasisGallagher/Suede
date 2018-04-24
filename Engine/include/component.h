#pragma once
#include "object.h"

SUEDE_DEFINE_OBJECT_POINTER(Entity);
SUEDE_DEFINE_OBJECT_POINTER(Component);

class SUEDE_API IComponent : virtual public IObject {
public:
	virtual void SetEntity(Entity entity) = 0;
	virtual Entity GetEntity() = 0;

	virtual void Update() = 0;
};
