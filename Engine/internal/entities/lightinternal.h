#include "light.h"
#include "internal/components/componentinternal.h"

class LightInternal : virtual public ILight, public ComponentInternal {
public:
	LightInternal(ObjectType type) : ComponentInternal(type), intensity_(1) {}
	~LightInternal() {}

	virtual void SetImportance(LightImportance value) { importance_ = value; }
	virtual LightImportance GetImportance() { return importance_; }

	virtual void SetColor(const glm::vec3 & value) { color_ = value; }
	virtual glm::vec3 GetColor() { return color_; }

	virtual void SetIntensity(float value) { intensity_ = value; }
	virtual float GetIntensity() { return intensity_; }

protected:
	float intensity_;
	glm::vec3 color_;
	LightImportance importance_;
};

class SpotLightInternal : public ISpotLight, public LightInternal {
public:
	SpotLightInternal() : LightInternal(ObjectType::SpotLight) {}
};

class PointLightInternal : public IPointLight, public LightInternal {
public:
	PointLightInternal() : LightInternal(ObjectType::PointLight) {}

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
public:
	DirectionalLightInternal() : LightInternal(ObjectType::DirectionalLight) {
		intensity_ = 0.5f;
	}
};
