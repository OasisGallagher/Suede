#include "material.h"
#include "environment.h"

class EnvironmentInternal : public IEnvironment {
public:
	EnvironmentInternal() : fogDensity_(0.05f) {}

public:
	virtual void SetSkybox(Material value);
	virtual Material GetSkybox() { return skybox_; }

	virtual void SetAmbientColor(const glm::vec3& value) { ambientColor_ = value; }
	virtual glm::vec3 GetAmbientColor() { return ambientColor_; }

	virtual void SetFogColor(const glm::vec3& value) { fogColor_ = value; }
	virtual glm::vec3 GetFogColor() { return fogColor_; }

	virtual void SetFogDensity(float value) { fogDensity_ = value; }
	virtual float GetFogDensity() { return fogDensity_; }

private:
	Material skybox_;
	glm::vec3 ambientColor_;

	float fogDensity_;
	glm::vec3 fogColor_;
};
