#include <glm/gtc/matrix_transform.hpp>

#include "tools/math2.h"
#include "projectorinternal.h"

ProjectorInternal::ProjectorInternal()
	: EntityInternal(ObjectTypeProjector), depth_(0) {
	aspect_ = 1.3f;
	near_ = 1.f;
	far_ = 1000.f;
	fieldOfView_ = Math::Pi() / 3.f;
	projection_ = glm::perspective(fieldOfView_, aspect_, near_, far_);
}

void ProjectorInternal::SetAspect(float value) {
	if (!Math::Approximately(aspect_, value)) {
		aspect_ = value;
		projection_ = glm::perspective(fieldOfView_, aspect_, near_, far_);
	}
}

void ProjectorInternal::SetNearClipPlane(float value) {
	if (!Math::Approximately(near_, value)) {
		near_ = value;
		projection_ = glm::perspective(fieldOfView_, aspect_, near_, far_);
	}
}

void ProjectorInternal::SetFarClipPlane(float value) {
	if (!Math::Approximately(far_, value)) {
		far_ = value;
		projection_ = glm::perspective(fieldOfView_, aspect_, near_, far_);
	}
}

void ProjectorInternal::SetFieldOfView(float value) {
	if (!Math::Approximately(fieldOfView_, value)) {
		fieldOfView_ = value;
		projection_ = glm::perspective(fieldOfView_, aspect_, near_, far_);
	}
}
