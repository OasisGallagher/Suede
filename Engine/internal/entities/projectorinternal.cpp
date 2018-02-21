#include <glm/gtc/matrix_transform.hpp>

#include "tools/math2.h"
#include "projectorinternal.h"

ProjectorInternal::ProjectorInternal()
	: EntityInternal(ObjectTypeProjector), depth_(0) {
	aspect_ = 1.3f;
	near_ = 1.f;
	far_ = 1000.f;
	fieldOfView_ = Math::Pi() / 3.f;
	Recalculate();
}

void ProjectorInternal::SetAspect(float value) {
	if (!Math::Approximately(aspect_, value)) {
		aspect_ = value;
		Recalculate();
	}
}

void ProjectorInternal::SetNearClipPlane(float value) {
	if (!Math::Approximately(near_, value)) {
		near_ = value;
		Recalculate();
	}
}

void ProjectorInternal::SetFarClipPlane(float value) {
	if (!Math::Approximately(far_, value)) {
		far_ = value;
		Recalculate();
	}
}

void ProjectorInternal::SetFieldOfView(float value) {
	if (!Math::Approximately(fieldOfView_, value)) {
		fieldOfView_ = value;
		Recalculate();
	}
}

void ProjectorInternal::Recalculate() {
	projection_ = glm::ortho(-5.f, 5.f, -5.f, 5.f, 1.f, 1000.f);
	//projection_ = glm::perspective(fieldOfView_, aspect_, near_, far_);
}
