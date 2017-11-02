#pragma once
#include "skybox.h"
#include "internal/sprites/spriteinternal.h"

class SkyboxInternal : public ISkybox, public SpriteInternal {
	DEFINE_FACTORY_METHOD(Skybox)

public:
	SkyboxInternal();

public:
	virtual bool Load(const std::string(&textures)[6]);
};
