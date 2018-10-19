#include "color.h"
#include "material.h"
#include "environment.h"

class EnvironmentInternal {
public:
	EnvironmentInternal() : fogDensity_(0.0001f) {}

public:
	void SetSkybox(Material value);
	Material GetSkybox() { return skybox_; }

	void SetAmbientColor(const Color& value) { ambientColor_ = value; }
	Color GetAmbientColor() { return ambientColor_; }

	void SetFogColor(const Color& value) { fogColor_ = value; }
	Color GetFogColor() { return fogColor_; }

	void SetFogDensity(float value) { fogDensity_ = value; }
	float GetFogDensity() { return fogDensity_; }

private:
	Material skybox_;
	Color ambientColor_;

	float fogDensity_;
	Color fogColor_;
};
