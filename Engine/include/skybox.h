#pragma once
#include "entity.h"

class SUEDE_API ISkybox : virtual public IEntity {
public:
	virtual bool Load(const std::string(&textures)[6]) = 0;
};

SUEDE_DEFINE_OBJECT_POINTER(Skybox);
SUEDE_DECLARE_OBJECT_CREATER(Skybox);
