#include "frustum.h"
#include "projectorinternal.h"

ProjectorInternal::ProjectorInternal()
	: EntityInternal(ObjectType::Projector), depth_(0) {
}
