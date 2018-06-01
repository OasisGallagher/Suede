#pragma once
#include <glm/glm.hpp>

#include "plane.h"
#include "screen.h"
#include "camera.h"
#include "frustum.h"
#include "frameeventlistener.h"
#include "internal/rendering/rendering.h"
#include "internal/entities/entityinternal.h"

//class GBuffer;

class Rendering;
class ImageEffect;
class GizmosPainter;

class Sample;
class CameraInternal : public ICamera, public EntityInternal, public Frustum, public ScreenSizeChangedListener, public FrameEventListener {
	DEFINE_FACTORY_METHOD(Camera)

public:
	CameraInternal();
	~CameraInternal();

public:
	virtual void SetDepth(int value);
	virtual int GetDepth() { return depth_;  }

	virtual void SetClearType(ClearType value) { rendering_->SetClearType(value);; }
	virtual ClearType GetClearType() { return rendering_->GetClearType(); }

	virtual void SetRenderPath(RenderPath value) { rendering_->SetRenderPath(value); }
	virtual RenderPath GetRenderPath() { return rendering_->GetRenderPath(); }

	virtual void SetDepthTextureMode(DepthTextureMode value) { rendering_->SetDepthTextureMode(value); }
	virtual DepthTextureMode GetDepthTextureMode() { return rendering_->GetDepthTextureMode(); }

	virtual void SetClearColor(const glm::vec3& value) { rendering_->SetClearColor(value); }
	virtual glm::vec3 GetClearColor() { return rendering_->GetClearColor(); }

	virtual void SetTargetTexture(RenderTexture value) { rendering_->SetTargetTexture(value); }
	virtual RenderTexture GetTargetTexture() { return rendering_->GetTargetTexture(); }

	virtual Texture2D Capture();

public:
	virtual void Update();
	virtual void Render();

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

	virtual void SetRect(const Rect& value);
	virtual const Rect& GetRect() const;

	virtual const glm::mat4& GetProjectionMatrix() { return Frustum::GetProjectionMatrix(); }

	virtual glm::vec3 WorldToScreenPoint(const glm::vec3& position);
	virtual glm::vec3 ScreenToWorldPoint(const glm::vec3& position);

public:
	virtual void AddImageEffect(ImageEffect* effect) { rendering_->AddImageEffect(effect); }
	virtual void AddGizmosPainter(GizmosPainter* painter) { gizmosPainters_.push_back(painter); }

public:
	virtual void OnScreenSizeChanged(uint width, uint height);

protected:
	virtual void OnProjectionMatrixChanged();

public:
	virtual int GetFrameEventQueue();
	virtual void OnFrameLeave();

private:
	bool IsMainCamera() const;
	void OnDrawGizmos();

private:
	int depth_;

	//GBuffer* gbuffer_;

	Plane planes_[6];
	Rendering* rendering_;
	
	std::vector<GizmosPainter*> gizmosPainters_;
};
