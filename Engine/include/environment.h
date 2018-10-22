#pragma once
#include "material.h"
#include "tools/singleton.h"

class SUEDE_API Environment : public Singleton2<Environment> {
	friend class Singleton<Environment>;
	SUEDE_DECLARE_IMPLEMENTATION(Environment)

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
