#pragma once

#include <glm/glm.hpp>

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

class SUEDE_API ILight : virtual public IComponent {
	SUEDE_DECLARE_COMPONENT()

public:
	virtual void SetType(LightType value) = 0;
	virtual LightType GetType() = 0;

	virtual void SetImportance(LightImportance value) = 0;
	virtual LightImportance GetImportance() = 0;

	virtual void SetColor(const Color& value) = 0;
	virtual Color GetColor() = 0;

	virtual void SetIntensity(float value) = 0;
	virtual float GetIntensity() = 0;

	virtual int GetUpdateStrategy() { return UpdateStrategyNone; }
};

SUEDE_DEFINE_OBJECT_POINTER(Light);
