#include "frustum.h"
#include "projectorinternal.h"

RTTI_CLASS_DEFINITION(IProjector, IComponent)

ProjectorInternal::ProjectorInternal()
	: ComponentInternal(ObjectType::Projector), depth_(0) {
}
