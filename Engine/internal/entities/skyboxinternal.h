#pragma once
#include "skybox.h"
#include "internal/entities/entityinternal.h"

class SkyboxInternal : public ISkybox, public EntityInternal {
	DEFINE_FACTORY_METHOD(Skybox)

public:
	SkyboxInternal();

public:
	virtual bool Load(const std::string(&textures)[6]);
};
