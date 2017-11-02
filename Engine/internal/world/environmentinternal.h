#include "environment.h"

class EnvironmentInternal : public IEnvironment {
public:
	virtual void SetSkybox(Skybox value) { skybox_ = value; }
	virtual Skybox GetSkybox() { return skybox_; }
	virtual void SetAmbientColor(const glm::vec3 & value) { ambientColor_ = value; }
	virtual glm::vec3 GetAmbientColor() { return ambientColor_; }

private:
	Skybox skybox_;
	glm::vec3 ambientColor_;
};
