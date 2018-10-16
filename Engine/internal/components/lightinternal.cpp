#include "lightinternal.h"

ILight::ILight() : IComponent(MEMORY_NEW(LightInternal)) {}
void ILight::SetType(LightType value) { dptr()->SetType(value); }
LightType ILight::GetType() { return dptr()->GetType(); }
void ILight::SetImportance(LightImportance value) { dptr()->SetImportance(value); }
LightImportance ILight::GetImportance() { return dptr()->GetImportance(); }
void ILight::SetColor(const Color& value) { dptr()->SetColor(value); }
Color ILight::GetColor() { return dptr()->GetColor(); }
void ILight::SetIntensity(float value) { dptr()->SetIntensity(value); }
float ILight::GetIntensity() { return dptr()->GetIntensity(); }
int ILight::GetUpdateStrategy() { return dptr()->GetUpdateStrategy(); }

SUEDE_DEFINE_COMPONENT(ILight, IComponent)

LightInternal::LightInternal() :
	ComponentInternal(ObjectType::Light), type_(LightType::Directional), intensity_(1) {

}
