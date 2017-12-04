#pragma once
#include "skybox.h"

class SUEDE_API IEnvironment {
public:
	virtual void SetSkybox(Skybox value) = 0;
	virtual Skybox GetSkybox() = 0;

	virtual void SetAmbientColor(const glm::vec3& value) = 0;
	virtual glm::vec3 GetAmbientColor() = 0;
};

SUEDE_DEFINE_OBJECT_POINTER(Environment);
SUEDE_DECLARE_OBJECT_CREATER(Environment);