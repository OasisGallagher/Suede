#include "frustum.h"
#include "projectorinternal.h"

ProjectorInternal::ProjectorInternal()
	: ComponentInternal(ObjectType::Projector), depth_(0) {
}
