#include "lightinternal.h"

Light::Light() : Component(MEMORY_NEW(LightInternal)) {}
void Light::SetType(LightType value) { _suede_dptr()->SetType(value); }
LightType Light::GetType() { return _suede_dptr()->GetType(); }
void Light::SetImportance(LightImportance value) { _suede_dptr()->SetImportance(value); }
LightImportance Light::GetImportance() { return _suede_dptr()->GetImportance(); }
void Light::SetColor(const Color& value) { _suede_dptr()->SetColor(value); }
Color Light::GetColor() { return _suede_dptr()->GetColor(); }
void Light::SetIntensity(float value) { _suede_dptr()->SetIntensity(value); }
float Light::GetIntensity() { return _suede_dptr()->GetIntensity(); }
int Light::GetUpdateStrategy() { return _suede_dptr()->GetUpdateStrategy(); }

SUEDE_DEFINE_COMPONENT_INTERNAL(Light, Component)

LightInternal::LightInternal() :
	ComponentInternal(ObjectType::Light), type_(LightType::Directional), intensity_(1) {

}
