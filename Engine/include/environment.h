#pragma once
class SUEDE_API IEnvironment {
public:
	virtual void SetSkybox(Material value) = 0;
	virtual Material GetSkybox() = 0;

	virtual void SetAmbientColor(const glm::vec3& value) = 0;
	virtual glm::vec3 GetAmbientColor() = 0;

	virtual void SetFogColor(const glm::vec3& value) = 0;
	virtual glm::vec3 GetFogColor() = 0;

	virtual void SetFogDensity(float value) = 0;
	virtual float GetFogDensity() = 0;
};

SUEDE_DEFINE_OBJECT_POINTER(Environment);
SUEDE_DECLARE_OBJECT_CREATER(Environment);
