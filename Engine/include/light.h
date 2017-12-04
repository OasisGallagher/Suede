#pragma once

#include <glm/glm.hpp>

#include "sprite.h"

enum LightImportance {
	LightImportanceNotImportant,
	LightImportanceImportant,
};

class ILight : virtual public ISprite {
public:
	virtual void SetImportance(LightImportance value) = 0;
	virtual LightImportance GetImportance() = 0;

	virtual void SetColor(const glm::vec3& value) = 0;
	virtual glm::vec3 GetColor() = 0;
};

class ISpotLight : virtual public ILight {
};

class IPointLight : virtual public ILight {
public:
	virtual void SetConstant(float value) = 0;
	virtual float GetConstant() = 0;

	virtual void SetLinear(float value) = 0;
	virtual float GetLinear() = 0;

	virtual void SetExp(float value) = 0;
	virtual float GetExp() = 0;
};

class IDirectionalLight : virtual public ILight {
};

SUEDE_DEFINE_OBJECT_POINTER(Light);
SUEDE_DEFINE_OBJECT_POINTER(SpotLight);
SUEDE_DEFINE_OBJECT_POINTER(PointLight);
SUEDE_DEFINE_OBJECT_POINTER(DirectionalLight);

SUEDE_DECLARE_OBJECT_CREATER(SpotLight);
SUEDE_DECLARE_OBJECT_CREATER(PointLight);
SUEDE_DECLARE_OBJECT_CREATER(DirectionalLight);
