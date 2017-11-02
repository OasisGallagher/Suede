#pragma once
#include <glm/glm.hpp>

#include "skybox.h"
#include "sprite.h"

enum ClearType {
	ClearTypeColor,
	ClearTypeSkybox,
};

class PostEffect;

class ENGINE_EXPORT ICamera : virtual public ISprite {
public:
	virtual void SetDepth(int value) = 0;
	virtual int GetDepth() = 0;

	virtual void SetClearType(ClearType value) = 0;
	virtual ClearType GetClearType() = 0;

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

	virtual void AddPostEffect(PostEffect* effect) = 0;

	virtual Texture2D Capture() = 0;

	// TODO: internal method.
	virtual void Render() = 0;
};

typedef std::shared_ptr<ICamera> Camera;
