#pragma once
#include "material.h"
#include "tools/singleton.h"

class SUEDE_API Environment : public Singleton2<Environment> {
	friend class Singleton2<Environment>;

public:
	void SetSkybox(Material value);
	Material GetSkybox();

	void SetAmbientColor(const Color& value);
	Color GetAmbientColor();

	void SetFogColor(const Color& value);
	Color GetFogColor();

	void SetFogDensity(float value);
	float GetFogDensity();

private:
	Environment();
};
