#include "color.h"
#include "material.h"
#include "environment.h"

class EnvironmentInternal : public Environment {
public:
	EnvironmentInternal() : fogDensity_(0.0001f) {}

public:
	virtual void SetSkybox(Material value);
	virtual Material GetSkybox() { return skybox_; }

	virtual void SetAmbientColor(const Color& value) { ambientColor_ = value; }
	virtual Color GetAmbientColor() { return ambientColor_; }

	virtual void SetFogColor(const Color& value) { fogColor_ = value; }
	virtual Color GetFogColor() { return fogColor_; }

	virtual void SetFogDensity(float value) { fogDensity_ = value; }
	virtual float GetFogDensity() { return fogDensity_; }

private:
	Material skybox_;
	Color ambientColor_;

	float fogDensity_;
	Color fogColor_;
};
