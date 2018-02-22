#include "frustum.h"
#include "tools/math2.h"
#include "debug/debug.h"
#include <glm/gtc/matrix_transform.hpp>

#define DEBUG_PROJECTION_MODE(name, perspective) \
	if (perspective != perspective_) { \
		Debug::LogError("%s is only valid in %s mode.", name, perspective ? "perspective" : "orthographic"); \
	} else (void)0

Frustum::Frustum() : perspective_(true) {
	near_ = 1.f;
	far_ = 1000.f;
	aspect_ = 1.3f;
	fieldOfView_ = Math::Pi() / 3.f;
	orthographicSize_ = 5;
	RecalculateProjectionMatrix();
}

void Frustum::RecalculateProjectionMatrix() {
	if (perspective_) {
		projection_ = glm::perspective(fieldOfView_, aspect_, near_, far_);
	}
	else {
		float hHalfSize = orthographicSize_ * aspect_;
		projection_ = glm::ortho(-hHalfSize, hHalfSize, -orthographicSize_, orthographicSize_, near_, far_);
	}
}

void Frustum::SetPerspective(bool value) {
	if (perspective_ != value) {
		perspective_ = value;
		RecalculateProjectionMatrix();
	}
}

float Frustum::GetOrthographicSize() const {
	DEBUG_PROJECTION_MODE("size", false);
	return orthographicSize_;
}

void Frustum::SetOrthographicSize(float value) {
	DEBUG_PROJECTION_MODE("size", false);
	if (!Math::Approximately(orthographicSize_, value)) {
		orthographicSize_ = value;
		RecalculateProjectionMatrix();
	}
}

void Frustum::SetAspect(float value) {
	if (!Math::Approximately(aspect_, value)) {
		aspect_ = value;
		RecalculateProjectionMatrix();
	}
}

void Frustum::SetNearClipPlane(float value) {
	if (!Math::Approximately(near_, value)) {
		near_ = value;
		RecalculateProjectionMatrix();
	}
}

void Frustum::SetFarClipPlane(float value) {
	if (!Math::Approximately(far_, value)) {
		far_ = value;
		RecalculateProjectionMatrix();
	}
}

void Frustum::SetFieldOfView(float value) {
	DEBUG_PROJECTION_MODE("fieldOfView", true);
	if (!Math::Approximately(fieldOfView_, value)) {
		fieldOfView_ = value;
		RecalculateProjectionMatrix();
	}
}

float Frustum::GetFieldOfView() const {
	DEBUG_PROJECTION_MODE("fieldOfView", true);
	return fieldOfView_;
}
