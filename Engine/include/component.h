#pragma once
#include "object.h"

SUEDE_DEFINE_OBJECT_POINTER(Entity);
SUEDE_DEFINE_OBJECT_POINTER(Transform);
SUEDE_DEFINE_OBJECT_POINTER(Component);

enum {
	UpdateStrategyNone = 0,
	UpdateStrategyCulling = 1,
	UpdateStrategyRendering = 1 << 1,
};

class SUEDE_API IComponent : virtual public IObject {
public:
	virtual void SetEntity(Entity entity) = 0;
	virtual Entity GetEntity() = 0;

	virtual Transform GetTransform() = 0;

	virtual void CullingUpdate() = 0;
	virtual void RenderingUpdate() = 0;

	virtual int GetUpdateStrategy() = 0;
};
