#pragma once
#include "projector.h"
#include "internal/entities/entityinternal.h"

class Frustum;

class ProjectorInternal : public IProjector, public EntityInternal {
	DEFINE_FACTORY_METHOD(Projector)

public:
	ProjectorInternal();
	~ProjectorInternal();

public:
	virtual Texture GetTexture() const { return texture_; }
	virtual void SetTexture(Texture value) { texture_ = value; }

	virtual void SetDepth(int value) { depth_ = value; }
	virtual int GetDepth() const { return depth_; }

	virtual bool GetPerspective() const;
	virtual void SetPerspective(bool value);

	virtual float GetOrthographicSize() const;
	virtual void SetOrthographicSize(float value);
	virtual void SetAspect(float value);
	virtual float GetAspect() const;

	virtual void SetNearClipPlane(float value);
	virtual float GetNearClipPlane() const;

	virtual void SetFarClipPlane(float value);
	virtual float GetFarClipPlane() const;

	virtual void SetFieldOfView(float value);
	virtual float GetFieldOfView() const;

	virtual const glm::mat4& GetProjectionMatrix() const;

private:
	int depth_;
	Texture texture_;
	Frustum* frustum_;
};
