#pragma once
#include "object.h"

SUEDE_DEFINE_OBJECT_POINTER(Entity);
SUEDE_DEFINE_OBJECT_POINTER(Component);

enum {
	UpdateStrategyNone = 0,
	UpdateStrategyCulling = 1,
	UpdateStrategyRendering = 2,
};

class SUEDE_API IComponent : virtual public IObject {
public:
	virtual void SetEntity(Entity entity) = 0;
	virtual Entity GetEntity() = 0;

	virtual void CullingUpdate() = 0;
	virtual void RenderingUpdate() = 0;

	virtual int GetUpdateStrategy() = 0;
};
