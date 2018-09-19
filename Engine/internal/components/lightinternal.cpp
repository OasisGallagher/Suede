#include "lightinternal.h"

SUEDE_DEFINE_COMPONENT(ILight, IComponent)

LightInternal::LightInternal() :
	ComponentInternal(ObjectType::Light), type_(LightType::Directional), intensity_(1) {

}
