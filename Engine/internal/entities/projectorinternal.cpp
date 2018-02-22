#include "frustum.h"
#include "projectorinternal.h"

ProjectorInternal::ProjectorInternal()
	: EntityInternal(ObjectTypeProjector), depth_(0) {
	frustum_ = MEMORY_CREATE(Frustum);
}

ProjectorInternal::~ProjectorInternal() {
	MEMORY_RELEASE(frustum_);
}

bool ProjectorInternal::GetPerspective() const {
	return frustum_->GetPerspective();
}

void ProjectorInternal::SetPerspective(bool value) {
	frustum_->SetPerspective(value);
}

float ProjectorInternal::GetOrthographicSize() const {
	return frustum_->GetOrthographicSize();
}

void ProjectorInternal::SetOrthographicSize(float value) {
	frustum_->SetOrthographicSize(value);
}

void ProjectorInternal::SetAspect(float value) {
	frustum_->SetAspect(value);
}

float ProjectorInternal::GetAspect() const {
	return frustum_->GetAspect();
}

void ProjectorInternal::SetNearClipPlane(float value) {
	frustum_->SetNearClipPlane(value);
}

float ProjectorInternal::GetNearClipPlane() const {
	return frustum_->GetNearClipPlane();
}

void ProjectorInternal::SetFarClipPlane(float value) {
	frustum_->SetFarClipPlane(value);
}

float ProjectorInternal::GetFarClipPlane() const {
	return frustum_->GetFarClipPlane();
}

void ProjectorInternal::SetFieldOfView(float value) {
	frustum_->SetFieldOfView(value);
}

float ProjectorInternal::GetFieldOfView() const {
	return frustum_->GetFieldOfView();
}

const glm::mat4& ProjectorInternal::GetProjectionMatrix() const {
	return frustum_->GetProjectionMatrix();
}
