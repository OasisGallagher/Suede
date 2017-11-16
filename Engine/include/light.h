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

DEFINE_OBJECT_PTR(Light);
DEFINE_OBJECT_PTR(SpotLight);
DEFINE_OBJECT_PTR(PointLight);
DEFINE_OBJECT_PTR(DirectionalLight);
