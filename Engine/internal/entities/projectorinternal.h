#pragma once
#include "projector.h"
#include "internal/entities/entityinternal.h"

class ProjectorInternal : public IProjector, public EntityInternal {
	DEFINE_FACTORY_METHOD(Projector)

public:
	ProjectorInternal();

public:
	virtual void SetDepth(int value) { depth_ = value; }
	virtual int GetDepth() { return depth_; }

	virtual void SetAspect(float value);
	virtual float GetAspect() { return aspect_; }

	virtual void SetNearClipPlane(float value);
	virtual float GetNearClipPlane() { return near_; }

	virtual void SetFarClipPlane(float value);
	virtual float GetFarClipPlane() { return far_; }

	virtual void SetFieldOfView(float value);
	virtual float GetFieldOfView() { return fieldOfView_; }

	virtual const glm::mat4 & GetProjectionMatrix() { return projection_; }

private:
	int depth_;
	float aspect_;
	float near_, far_;
	float fieldOfView_;
	glm::mat4 projection_;
};