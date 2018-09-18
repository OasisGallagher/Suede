#include "light.h"
#include "internal/components/componentinternal.h"

class LightInternal : virtual public ILight, public ComponentInternal {
public:
	LightInternal(ObjectType type) : ComponentInternal(type), intensity_(1) {}
	~LightInternal() {}

	virtual void SetImportance(LightImportance value) { importance_ = value; }
	virtual LightImportance GetImportance() { return importance_; }

	virtual void SetColor(const Color& value) { color_ = value; }
	virtual Color GetColor() { return color_; }

	virtual void SetIntensity(float value) { intensity_ = value; }
	virtual float GetIntensity() { return intensity_; }

protected:
	Color color_;
	float intensity_;
	LightImportance importance_;
};

class SpotLightInternal : public ISpotLight, public LightInternal {
	DEFINE_FACTORY_METHOD(SpotLight)

public:
	SpotLightInternal() : LightInternal(ObjectType::SpotLight) {}
};

class PointLightInternal : public IPointLight, public LightInternal {
	DEFINE_FACTORY_METHOD(PointLight)

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
	DEFINE_FACTORY_METHOD(DirectionalLight)

public:
	DirectionalLightInternal() : LightInternal(ObjectType::DirectionalLight) {
		intensity_ = 0.5f;
	}
};
