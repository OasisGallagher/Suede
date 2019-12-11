#pragma once
#include <ZThread/Mutex.h>

#include "plane.h"
#include "screen.h"
#include "camera.h"
#include "frustum.h"

#include <ZThread/ThreadedExecutor.h>
#include "internal/culling/culling.h"
#include "internal/rendering/rendering.h"

#include "internal/components/componentinternal.h"

//class GBuffer;

class Sample;
class CameraInternal : public ComponentInternal, public Frustum
	, public CullingListener/*, public RenderingListener */
	, public ScreenSizeChangedListener {

public:
	CameraInternal();
	~CameraInternal();

public:
	void Awake();

	void SetDepth(Camera* self, int value);
	int GetDepth() const { return depth_;  }

	void SetRect(const Rect& value);
	const Rect& GetRect() const { return p_.normalizedRect; }

	void SetClearType(ClearType value) { p_.clearType = value; }
	ClearType GetClearType() const { return p_.clearType; }

	void SetRenderPath(RenderPath value) { p_.renderPath = value; }
	RenderPath GetRenderPath() const { return p_.renderPath; }

	void SetDepthTextureMode(DepthTextureMode value) { p_.depthTextureMode = value; }
	DepthTextureMode GetDepthTextureMode() const { return p_.depthTextureMode; }

	void SetClearColor(const Color& value) { p_.clearColor = value; }
	Color GetClearColor() const { return p_.clearColor; }

	void SetTargetTexture(RenderTexture* value) { p_.renderTextures.target = value; }
	RenderTexture* GetTargetTexture() { return p_.renderTextures.target.get(); }

	ref_ptr<Texture2D> Capture();

public:
	void Render();
	void OnBeforeWorldDestroyed();

public:
	bool GetPerspective() const { return Frustum::GetPerspective(); }
	void SetPerspective(bool value) { return Frustum::SetPerspective(value); }

	float GetOrthographicSize() const { return Frustum::GetOrthographicSize(); }
	void SetOrthographicSize(float value) { return Frustum::SetOrthographicSize(value); }

	void SetAspect(float value) { return Frustum::SetAspect(value); }
	float GetAspect() const { return Frustum::GetAspect(); }

	void SetNearClipPlane(float value) { return Frustum::SetNearClipPlane(value); }
	float GetNearClipPlane() const { return Frustum::GetNearClipPlane(); }

	void SetFarClipPlane(float value) { return Frustum::SetFarClipPlane(value); }
	float GetFarClipPlane() const { return Frustum::GetFarClipPlane(); }

	void SetFieldOfView(float value) { return Frustum::SetFieldOfView(value); }
	float GetFieldOfView() const { return Frustum::GetFieldOfView(); }

	const Matrix4& GetProjectionMatrix() { return Frustum::GetProjectionMatrix(); }

	void GetVisibleGameObjects(std::vector<GameObject*>& gameObjects);

	Vector3 WorldToScreenPoint(const Vector3& position);
	Vector3 ScreenToWorldPoint(const Vector3& position);

public:
	void OnScreenSizeChanged(uint width, uint height);

public:
	int GetUpdateStrategy() { return UpdateStrategyNone; }

protected:
	void OnProjectionMatrixChanged();

protected:
	void OnCullingFinished();
	//void OnRenderingFinished();

private:
	void CancelThreads();
	bool IsValidViewportRect();

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

	ZThread::Mutex visibleGameObjectsMutex_;
	std::vector<GameObject*> visibleGameObjects_;
};
