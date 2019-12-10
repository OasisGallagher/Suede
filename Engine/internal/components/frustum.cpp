#include "frustum.h"
#include "math/mathf.h"
#include "debug/debug.h"

Frustum::Frustum() : perspective_(true), matrixDirty_(true) {
	near_ = 1.f;
	far_ = 1000.f;
	aspect_ = 1.3f;
	fieldOfView_ = Mathf::Pi() / 3.f;
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
	if (!Mathf::Approximately(orthographicSize_, value)) {
		orthographicSize_ = value;
		DirtyProjectionMatrix();
	}
}

void Frustum::SetAspect(float value) {
	if (!Mathf::Approximately(aspect_, value)) {
		aspect_ = value;
		DirtyProjectionMatrix();
	}
}

void Frustum::SetNearClipPlane(float value) {
	if (!Mathf::Approximately(near_, value)) {
		near_ = value;
		DirtyProjectionMatrix();
	}
}

void Frustum::SetFarClipPlane(float value) {
	if (!Mathf::Approximately(far_, value)) {
		far_ = value;
		DirtyProjectionMatrix();
	}
}

void Frustum::SetFieldOfView(float value) {
	if (!Mathf::Approximately(fieldOfView_, value)) {
		fieldOfView_ = value;
		DirtyProjectionMatrix();
	}
}

float Frustum::GetFieldOfView() const {
	return fieldOfView_;
}

void Frustum::CalculateProjectionMatrix() {
	if (perspective_) {
		projection_ = Matrix4::Perspective(fieldOfView_, aspect_, near_, far_);
	}
	else {
		float hHalfSize = orthographicSize_ * aspect_;
		projection_ = Matrix4::Ortho(-hHalfSize, hHalfSize, -orthographicSize_, orthographicSize_, near_, far_);
	}

	matrixDirty_ = false;
	OnProjectionMatrixChanged();
}

const Matrix4& Frustum::GetProjectionMatrix() {
	if (matrixDirty_) { CalculateProjectionMatrix(); }
	return projection_;
}
