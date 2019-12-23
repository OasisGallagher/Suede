#pragma once
#include <mutex>

#include "plane.h"
#include "screen.h"
#include "camera.h"
#include "frustum.h"

#include "internal/culling/culling.h"
#include "internal/rendering/rendering.h"

#include "internal/components/componentinternal.h"

//class GBuffer;

class Sample;
class RenderingContext;
class CameraInternal : public ComponentInternal, public Frustum {
public:
	CameraInternal();
	~CameraInternal();

public:
	void Awake();

	void SetDepth(Camera* self, int value);
	int GetDepth() const { return depth_;  }

	void SetRect(const Rect& value) { normalizedRect_ = value; }
	const Rect& GetRect() const { return normalizedRect_; }

	void SetClearType(ClearType value) { clearType_ = value; }
	ClearType GetClearType() const { return clearType_; }

	void SetRenderPath(RenderPath value) { renderPath_ = value; }
	RenderPath GetRenderPath() const { return renderPath_; }

	void SetDepthTextureMode(DepthTextureMode value) { depthTextureMode_ = value; }
	DepthTextureMode GetDepthTextureMode() const { return depthTextureMode_; }

	void SetClearColor(const Color& value) { clearColor_ = value; }
	Color GetClearColor() const { return clearColor_; }

	void SetTargetTexture(RenderTexture* value) { targetTexture_ = value; }
	RenderTexture* GetTargetTexture() { return targetTexture_.get(); }

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
	int GetUpdateStrategy() { return UpdateStrategyNone; }

protected:
	void OnProjectionMatrixChanged();

private:
	void UpdateFrameState();

	bool IsValidViewportRect();

	void OnCullingFinished();
	void OnScreenSizeChanged(uint width, uint height);

private:
	int depth_;

	Plane planes_[6];

	Culling* culling_;

	bool pipelineReady_;
	PipelineBuilder* pipelineBuilder_;
	RenderingPipelines* frontPipelines_, *backPipelines_;

	Rendering* rendering_;

	Rect normalizedRect_;

	ClearType clearType_;
	Color clearColor_;

	DepthTextureMode depthTextureMode_;
	RenderPath renderPath_;

	ref_ptr<RenderTexture> targetTexture_;

	std::mutex visibleGameObjectsMutex_;
	std::vector<GameObject*> visibleGameObjects_;
};
