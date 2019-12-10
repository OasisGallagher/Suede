#pragma once

#include "color.h"
#include "component.h"
#include "tools/enum.h"

BETTER_ENUM(LightImportance, int,
	NotImportant,
	Important
)

BETTER_ENUM(LightType, int,
	Spot,
	Point,
	Directional
)

class SUEDE_API ILight : public IComponent {
	SUEDE_DECLARE_COMPONENT()
	SUEDE_DEFINE_METATABLE_NAME(Light)
	SUEDE_DECLARE_IMPLEMENTATION(Light)

public:
	ILight();

public:
	void SetType(LightType value);
	LightType GetType();

	void SetImportance(LightImportance value);
	LightImportance GetImportance();

	void SetColor(const Color& value);
	Color GetColor();

	void SetIntensity(float value);
	float GetIntensity();

	int GetUpdateStrategy();
};

SUEDE_DEFINE_OBJECT_POINTER(Light)
