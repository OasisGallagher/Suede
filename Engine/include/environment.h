#pragma once
#include "material.h"
#include "tools/singleton.h"

class SUEDE_API Environment : public Singleton2<Environment> {
public:
	virtual void SetSkybox(Material value) = 0;
	virtual Material GetSkybox() = 0;

	virtual void SetAmbientColor(const Color& value) = 0;
	virtual Color GetAmbientColor() = 0;

	virtual void SetFogColor(const Color& value) = 0;
	virtual Color GetFogColor() = 0;

	virtual void SetFogDensity(float value) = 0;
	virtual float GetFogDensity() = 0;
};
