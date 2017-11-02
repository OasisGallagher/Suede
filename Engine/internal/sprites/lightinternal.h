#include "light.h"
#include "internal/sprites/spriteinternal.h"

class LightInternal : virtual public ILight, public SpriteInternal {
public:
	LightInternal(ObjectType type) : SpriteInternal(type) {
	}

	virtual void SetImportance(LightImportance value) { importance_ = value; }
	virtual LightImportance GetImportance() { return importance_; }

	virtual void SetColor(const glm::vec3 & value) { color_ = value; }
	virtual glm::vec3 GetColor() { return color_; }

private:
	glm::vec3 color_;
	LightImportance importance_;
};

class SpotLightInternal : public ISpotLight, public LightInternal {
	DEFINE_FACTORY_METHOD(SpotLight)

public:
	SpotLightInternal() : LightInternal(ObjectTypeSpotLight) {}
};

class PointLightInternal : public IPointLight, public LightInternal {
	DEFINE_FACTORY_METHOD(PointLight)

public:
	PointLightInternal() : LightInternal(ObjectTypePointLight) {}

public:
	virtual void SetConstant(float value) { constant_ = value; }
	virtual float GetConstant() { return constant_; }

	virtual void SetLinear(float value) { linear_ = value; }
	virtual float GetLinear() { return linear_; }

	virtual void SetExp(float value) { exp_ = value; }
	virtual float GetExp() { return exp_; }

private:
	float exp_;
	float linear_;
	float constant_;
};

class DirectionalLightInternal : public IDirectionalLight, public LightInternal {
	DEFINE_FACTORY_METHOD(DirectionalLight)

public:
	DirectionalLightInternal() : LightInternal(ObjectTypeDirectionalLight) {}
};
