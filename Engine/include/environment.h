#pragma once
#include "skybox.h"

class ENGINE_EXPORT IEnvironment {
public:
	virtual void SetSkybox(Skybox value) = 0;
	virtual Skybox GetSkybox() = 0;

	virtual void SetAmbientColor(const glm::vec3& value) = 0;
	virtual glm::vec3 GetAmbientColor() = 0;
};

typedef std::shared_ptr<IEnvironment> Environment;
