#include "light.h"
#include "internal/components/componentinternal.h"

class LightInternal : public ComponentInternal {
public:
	LightInternal();
	~LightInternal() {}

public:
	void SetType(LightType value) { type_ = value; }
	LightType GetType() { return type_; }

	void SetImportance(LightImportance value) { importance_ = value; }
	LightImportance GetImportance() { return importance_; }

	void SetColor(const Color& value) { color_ = value; }
	Color GetColor() { return color_; }

	void SetIntensity(float value) { intensity_ = value; }
	float GetIntensity() { return intensity_; }

	int GetUpdateStrategy() { return UpdateStrategyNone; }

protected:
	Color color_;
	LightType type_;
	float intensity_;
	LightImportance importance_;
};
