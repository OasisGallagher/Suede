#include "material.h"
#include "environment.h"

class EnvironmentInternal : public IEnvironment {
public:
	virtual void SetSkybox(Material value);
	virtual Material GetSkybox() { return skybox_; }
	virtual void SetAmbientColor(const glm::vec3 & value) { ambientColor_ = value; }
	virtual glm::vec3 GetAmbientColor() { return ambientColor_; }

private:
	Material skybox_;
	glm::vec3 ambientColor_;
};
