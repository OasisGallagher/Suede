#pragma once
#include <glm/glm.hpp>

#include "rect.h"
#include "entity.h"
#include "tools/enum.h"

BETTER_ENUM(ClearType, int,
	Color,
	Skybox
)

BETTER_ENUM(RenderPath, int,
	Forward,
	Deferred
)

BETTER_ENUM(DepthTextureMode, int,
	None,
	Depth = 1
)

class ImageEffect;
class GizmosPainter;

class SUEDE_API ICamera : virtual public IEntity {
public:
	virtual void SetDepth(int value) = 0;
	virtual int GetDepth() = 0;

	virtual bool GetPerspective() const = 0;
	virtual void SetPerspective(bool value) = 0;

	/**
	 * @return half-size when in orthographic mode.
	 */
	virtual float GetOrthographicSize() const = 0;
	virtual void SetOrthographicSize(float value) = 0;

	virtual void SetClearType(ClearType value) = 0;
	virtual ClearType GetClearType() = 0;

	virtual void SetRenderPath(RenderPath value) = 0;
	virtual RenderPath GetRenderPath() = 0;

	virtual void SetDepthTextureMode(DepthTextureMode value) = 0;
	virtual DepthTextureMode GetDepthTextureMode() = 0;

	virtual void SetClearColor(const glm::vec3& value) = 0;
	virtual glm::vec3 GetClearColor() = 0;

	virtual void SetTargetTexture(RenderTexture value) = 0;
	virtual RenderTexture GetTargetTexture() = 0;

	virtual void SetAspect(float value) = 0;
	virtual float GetAspect() const = 0;

	virtual void SetNearClipPlane(float value) = 0;
	virtual float GetNearClipPlane() const = 0;

	virtual void SetFarClipPlane(float value) = 0;
	virtual float GetFarClipPlane() const = 0;
	
	virtual void SetFieldOfView(float value) = 0;
	virtual float GetFieldOfView() const = 0;

	virtual void SetRect(const Rect& value) = 0;
	virtual const Rect& GetRect() const = 0;

	virtual const glm::mat4& GetProjectionMatrix() = 0;

	virtual glm::vec3 WorldToScreenPoint(const glm::vec3& position) = 0;
	virtual glm::vec3 ScreenToWorldPoint(const glm::vec3& position) = 0;

	virtual void AddImageEffect(ImageEffect* effect) = 0;
	virtual void AddGizmosPainter(GizmosPainter* painter) = 0;

	virtual Texture2D Capture() = 0;

	virtual void Render() = 0;
	virtual void OnBeforeWorldDestroyed() = 0;
};

SUEDE_DEFINE_CUSTOM_OBJECT_POINTER(Camera) {
	SUEDE_IMPLEMENT_CUSTOM_OBJECT_POINTER(Camera)

	static void SetMain(Camera value);
	static Camera GetMain();
};

SUEDE_DECLARE_OBJECT_CREATER(Camera);
