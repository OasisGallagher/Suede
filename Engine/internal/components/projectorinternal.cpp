#include "frustum.h"
#include "projectorinternal.h"

SUEDE_DEFINE_COMPONENT(IProjector, IComponent)

ProjectorInternal::ProjectorInternal()
	: ComponentInternal(ObjectType::Projector), depth_(0) {
}
