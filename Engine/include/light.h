#pragma once

#include <glm/glm.hpp>

#include "component.h"
#include "tools/enum.h"

BETTER_ENUM(LightImportance, int,
	NotImportant,
	Important
)

class SUEDE_API ILight : virtual public IComponent {
	RTTI_CLASS_DECLARATION(ILight, IComponent)

public:
	virtual void SetImportance(LightImportance value) = 0;
	virtual LightImportance GetImportance() = 0;

	virtual void SetColor(const glm::vec3& value) = 0;
	virtual glm::vec3 GetColor() = 0;

	virtual void SetIntensity(float value) = 0;
	virtual float GetIntensity() = 0;

	virtual int GetUpdateStrategy() { return UpdateStrategyNone; }
};

class SUEDE_API ISpotLight : virtual public ILight {
	RTTI_CLASS_DECLARATION(ISpotLight, ILight)
};

class SUEDE_API IPointLight : virtual public ILight {
	RTTI_CLASS_DECLARATION(IPointLight, ILight)

public:
	virtual void SetConstant(float value) = 0;
	virtual float GetConstant() = 0;

	virtual void SetLinear(float value) = 0;
	virtual float GetLinear() = 0;

	virtual void SetExp(float value) = 0;
	virtual float GetExp() = 0;
};

class SUEDE_API IDirectionalLight : virtual public ILight {
	RTTI_CLASS_DECLARATION(IDirectionalLight, ILight)
};

SUEDE_DEFINE_OBJECT_POINTER(Light);
SUEDE_DEFINE_OBJECT_POINTER(SpotLight);
SUEDE_DEFINE_OBJECT_POINTER(PointLight);
SUEDE_DEFINE_OBJECT_POINTER(DirectionalLight);
