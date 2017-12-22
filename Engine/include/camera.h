#pragma once
#include <glm/glm.hpp>

#include "skybox.h"
#include "sprite.h"

enum ClearType {
	ClearTypeColor,
	ClearTypeSkybox,
};

enum RenderPath {
	RenderPathForward,
	RenderPathDeferred,
};

enum DepthTextureMode {
	DepthTextureModeNone,
	DepthTextureModeDepth = 1,
};

class ImageEffect;

class SUEDE_API ICamera : virtual public ISprite {
public:
	virtual void SetDepth(int value) = 0;
	virtual int GetDepth() = 0;

	virtual void SetClearType(ClearType value) = 0;
	virtual ClearType GetClearType() = 0;

	virtual void SetRenderPath(RenderPath value) = 0;
	virtual RenderPath GetRenderPath() = 0;

	virtual void SetDepthTextureMode(DepthTextureMode value) = 0;
	virtual DepthTextureMode GetDepthTextureMode() = 0;

	virtual void SetSkybox(Skybox value) = 0;
	virtual Skybox GetSkybox() = 0;

	virtual void SetClearColor(const glm::vec3& value) = 0;
	virtual glm::vec3 GetClearColor() = 0;

	virtual void SetRenderTexture(RenderTexture value) = 0;
	virtual RenderTexture GetRenderTexture() = 0;

	virtual void SetAspect(float value) = 0;
	virtual void SetNearClipPlane(float value) = 0;
	virtual void SetFarClipPlane(float value) = 0;
	virtual void SetFieldOfView(float value) = 0;

	virtual float GetAspect() = 0;
	virtual float GetNearClipPlane() = 0;
	virtual float GetFarClipPlane() = 0;
	virtual float GetFieldOfView() = 0;

	virtual const glm::mat4& GetProjectionMatrix() = 0;

	virtual glm::vec3 WorldToScreenPoint(const glm::vec3& position) = 0;
	virtual glm::vec3 ScreenToWorldPoint(const glm::vec3& position) = 0;

	virtual void AddImageEffect(ImageEffect* effect) = 0;

	virtual Texture2D Capture() = 0;

	virtual void Render() = 0;
};

SUEDE_DEFINE_OBJECT_POINTER(Camera);
SUEDE_DECLARE_OBJECT_CREATER(Camera);
