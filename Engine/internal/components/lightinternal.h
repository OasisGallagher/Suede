#include "light.h"
#include "internal/components/componentinternal.h"

class LightInternal : virtual public ILight, public ComponentInternal {
	DEFINE_FACTORY_METHOD(Light)

public:
	LightInternal();
	~LightInternal() {}

	virtual void SetType(LightType value) { type_ = value; }
	virtual LightType GetType() { return type_; }

	virtual void SetImportance(LightImportance value) { importance_ = value; }
	virtual LightImportance GetImportance() { return importance_; }

	virtual void SetColor(const Color& value) { color_ = value; }
	virtual Color GetColor() { return color_; }

	virtual void SetIntensity(float value) { intensity_ = value; }
	virtual float GetIntensity() { return intensity_; }

protected:
	Color color_;
	LightType type_;
	float intensity_;
	LightImportance importance_;
};
