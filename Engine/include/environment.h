#pragma once
#include "material.h"

class SUEDE_API Environment {
public:
	static void SetSkybox(Material* value);
	static Material* GetSkybox();

	static void SetAmbientColor(const Color& value);
	static Color GetAmbientColor();

	static void SetFogColor(const Color& value);
	static Color GetFogColor();

	static void SetFogDensity(float value);
	static float GetFogDensity();
};
