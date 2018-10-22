#include "lightinternal.h"

ILight::ILight() : IComponent(MEMORY_NEW(LightInternal)) {}
void ILight::SetType(LightType value) { _suede_dptr()->SetType(value); }
LightType ILight::GetType() { return _suede_dptr()->GetType(); }
void ILight::SetImportance(LightImportance value) { _suede_dptr()->SetImportance(value); }
LightImportance ILight::GetImportance() { return _suede_dptr()->GetImportance(); }
void ILight::SetColor(const Color& value) { _suede_dptr()->SetColor(value); }
Color ILight::GetColor() { return _suede_dptr()->GetColor(); }
void ILight::SetIntensity(float value) { _suede_dptr()->SetIntensity(value); }
float ILight::GetIntensity() { return _suede_dptr()->GetIntensity(); }
int ILight::GetUpdateStrategy() { return _suede_dptr()->GetUpdateStrategy(); }

SUEDE_DEFINE_COMPONENT(ILight, IComponent)

LightInternal::LightInternal() :
	ComponentInternal(ObjectType::Light), type_(LightType::Directional), intensity_(1) {

}
