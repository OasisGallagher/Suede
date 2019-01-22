#include "frustum.h"
#include "tools/math2.h"
#include "debug/debug.h"
#include <glm/gtc/matrix_transform.hpp>

Frustum::Frustum() : perspective_(true), matrixDirty_(true) {
	near_ = 1.f;
	far_ = 1000.f;
	aspect_ = 1.3f;
	fieldOfView_ = Math::Pi / 3.f;
	orthographicSize_ = 5;
	DirtyProjectionMatrix();
}

void Frustum::DirtyProjectionMatrix() {
	matrixDirty_ = true;
}

void Frustum::SetPerspective(bool value) {
	if (perspective_ != value) {
		perspective_ = value;
		DirtyProjectionMatrix();
	}
}

float Frustum::GetOrthographicSize() const {
	return orthographicSize_;
}

void Frustum::SetOrthographicSize(float value) {
	if (!Math::Approximately(orthographicSize_, value)) {
		orthographicSize_ = value;
		DirtyProjectionMatrix();
	}
}

void Frustum::SetAspect(float value) {
	if (!Math::Approximately(aspect_, value)) {
		aspect_ = value;
		DirtyProjectionMatrix();
	}
}

void Frustum::SetNearClipPlane(float value) {
	if (!Math::Approximately(near_, value)) {
		near_ = value;
		DirtyProjectionMatrix();
	}
}

void Frustum::SetFarClipPlane(float value) {
	if (!Math::Approximately(far_, value)) {
		far_ = value;
		DirtyProjectionMatrix();
	}
}

void Frustum::SetFieldOfView(float value) {
	if (!Math::Approximately(fieldOfView_, value)) {
		fieldOfView_ = value;
		DirtyProjectionMatrix();
	}
}

float Frustum::GetFieldOfView() const {
	return fieldOfView_;
}

void Frustum::CalculateProjectionMatrix() {
	if (perspective_) {
		projection_ = glm::perspective(fieldOfView_, aspect_, near_, far_);
	}
	else {
		float hHalfSize = orthographicSize_ * aspect_;
		projection_ = glm::ortho(-hHalfSize, hHalfSize, -orthographicSize_, orthographicSize_, near_, far_);
	}

	matrixDirty_ = false;
	OnProjectionMatrixChanged();
}

const glm::mat4& Frustum::GetProjectionMatrix() {
	if (matrixDirty_) { CalculateProjectionMatrix(); }
	return projection_;
}
