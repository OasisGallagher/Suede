#pragma once
#include "sprite.h"

class ENGINE_EXPORT ISkybox : virtual public ISprite {
public:
	virtual bool Load(const std::string(&textures)[6]) = 0;
};

typedef std::shared_ptr<ISkybox> Skybox;
