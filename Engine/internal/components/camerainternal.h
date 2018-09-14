#pragma once
#include <glm/glm.hpp>
#include <ZThread/Mutex.h>

#include "plane.h"
#include "screen.h"
#include "camera.h"
#include "frustum.h"
#include "frameeventlistener.h"

#include <ZThread/ThreadedExecutor.h>
#include "internal/culling/culling.h"
#include "internal/rendering/rendering.h"

#include "internal/components/componentinternal.h"

//class GBuffer;

class ImageEffect;
class GizmosPainter;

class Sample;
class CameraInternal : public ICamera
	, public ComponentInternal, public Frustum
	, public CullingListener/*, public RenderingListener */
	, public ScreenSizeChangedListener, public FrameEventListener {
	DEFINE_FACTORY_METHOD(Camera)

public:
	CameraInternal();
	~CameraInternal();

public:
	virtual void SetDepth(int value);
	virtual int GetDepth() { return depth_;  }

	virtual void SetRect(const Rect& value);
	virtual const Rect& GetRect() const { return p_.normalizedRect; }

	virtual void SetClearType(ClearType value) { p_.clearType = value; }
	virtual ClearType GetClearType() { return p_.clearType; }

	virtual void SetRenderPath(RenderPath value) { p_.renderPath = value; }
	virtual RenderPath GetRenderPath() { return p_.renderPath; }

	virtual void SetDepthTextureMode(DepthTextureMode value) { p_.depthTextureMode = value; }
	virtual DepthTextureMode GetDepthTextureMode() { return p_.depthTextureMode; }

	virtual void SetClearColor(const glm::vec3& value) { p_.clearColor = value; }
	virtual glm::vec3 GetClearColor() { return p_.clearColor; }

	virtual void SetTargetTexture(RenderTexture value) { p_.renderTextures.target; }
	virtual RenderTexture GetTargetTexture() { return p_.renderTextures.target; }

	virtual Texture2D Capture();

public:
	virtual void Render();
	virtual void OnBeforeWorldDestroyed();

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

	virtual void GetVisibleEntities(std::vector<GameObject>& entities);

	virtual glm::vec3 WorldToScreenPoint(const glm::vec3& position);
	virtual glm::vec3 ScreenToWorldPoint(const glm::vec3& position);

public:
	virtual void AddImageEffect(ImageEffect* effect) { p_.imageEffects.push_back(effect); }
	virtual void AddGizmosPainter(GizmosPainter* painter) { gizmosPainters_.push_back(painter); }

public:
	virtual void OnScreenSizeChanged(uint width, uint height);

public:
	virtual int GetUpdateStrategy() { return UpdateStrategyNone; }

protected:
	virtual void OnProjectionMatrixChanged();

protected:
	virtual void OnCullingFinished();
	//virtual void OnRenderingFinished();

public:
	virtual int GetFrameEventQueue();
	virtual void OnFrameLeave();

private:
	bool IsMainCamera() const;
	void OnDrawGizmos();
	void CancelThreads();

private:
	int depth_;

	//GBuffer* gbuffer_;

	Plane planes_[6];
	RenderingParameters p_;

	Culling* culling_;
	ZThread::Thread* cullingThread_;

	bool traitsReady_;
	RenderableTraits* traits0_, *traits1_;

	Rendering* rendering_;

	ZThread::Mutex visibleEntitiesMutex_;
	std::vector<GameObject> visibleEntities_;
	std::vector<GizmosPainter*> gizmosPainters_;
};
