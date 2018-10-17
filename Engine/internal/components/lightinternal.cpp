#include "lightinternal.h"

ILight::ILight() : IComponent(MEMORY_NEW(LightInternal)) {}
void ILight::SetType(LightType value) { _dptr()->SetType(value); }
LightType ILight::GetType() { return _dptr()->GetType(); }
void ILight::SetImportance(LightImportance value) { _dptr()->SetImportance(value); }
LightImportance ILight::GetImportance() { return _dptr()->GetImportance(); }
void ILight::SetColor(const Color& value) { _dptr()->SetColor(value); }
Color ILight::GetColor() { return _dptr()->GetColor(); }
void ILight::SetIntensity(float value) { _dptr()->SetIntensity(value); }
float ILight::GetIntensity() { return _dptr()->GetIntensity(); }
int ILight::GetUpdateStrategy() { return _dptr()->GetUpdateStrategy(); }

SUEDE_DEFINE_COMPONENT(ILight, IComponent)

LightInternal::LightInternal() :
	ComponentInternal(ObjectType::Light), type_(LightType::Directional), intensity_(1) {

}
