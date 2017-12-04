#pragma once
#include "sprite.h"

class SUEDE_API ISkybox : virtual public ISprite {
public:
	virtual bool Load(const std::string(&textures)[6]) = 0;
};

SUEDE_DEFINE_OBJECT_POINTER(Skybox);
SUEDE_DECLARE_OBJECT_CREATER(Skybox);
