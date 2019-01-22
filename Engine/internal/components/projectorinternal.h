#pragma once
#include "frustum.h"
#include "projector.h"
#include "internal/components/componentinternal.h"

class ProjectorInternal : public ComponentInternal, public Frustum {
	SUEDE_DECLARE_SELF_TYPE(IProjector)

public:	ProjectorInternal(IProjector* self);

public:
	virtual Texture GetTexture() const { return texture_; }
	virtual void SetTexture(Texture value) { texture_ = value; }

	virtual void SetDepth(int value) { depth_ = value; }
	virtual int GetDepth() const { return depth_; }

public:
	virtual bool GetPerspective() const { return Frustum::GetPerspective(); }
	virtual void SetPerspective(bool value) { return Frustum::SetPerspective(value); }

	virtual float GetOrthographicSize() const { return Frustum::GetOrthographicSize(); }
	virtual void SetOrthographicSize(float value) { return Frustum::SetOrthographicSize(value); }

	virtual void SetAspect(float value) { return Frustum::SetAspect(value); }
	virtual float GetAspect() const { return Frustum::GetAspect(); }

	virtual void SetNearClipPlane(float value) { return Frustum::SetNearClipPlane(value); }
	virtual float GetNearClipPlane() const { return Frustum::GetNearClipPlane(); }

	virtual void SetFarClipPlane(float value) { return Frustum::SetFarClipPlane(value); }
	virtual float GetFarClipPlane() const { return Frustum::GetFarClipPlane(); }

	virtual void SetFieldOfView(float value) { return Frustum::SetFieldOfView(value); }
	virtual float GetFieldOfView() const { return Frustum::GetFieldOfView(); }

	virtual const glm::mat4& GetProjectionMatrix() { return Frustum::GetProjectionMatrix(); }

public:
	virtual int GetUpdateStrategy() { return UpdateStrategyNone; }

private:
	int depth_;
	Texture texture_;
};
