#pragma once
#include <glm/glm.hpp>

#include "rect.h"
#include "color.h"
#include "texture.h"
#include "component.h"
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

SUEDE_DEFINE_OBJECT_POINTER(GameObject)

class SUEDE_API ICamera : public IComponent {
	SUEDE_DECLARE_COMPONENT()
	SUEDE_DEFINE_METATABLE_NAME(Camera)
	SUEDE_DECLARE_IMPLEMENTATION(Camera)

public:
	ICamera();

public:
	void SetDepth(int vaue);
	int GetDepth() const;

	bool GetPerspective() const;
	void SetPerspective(bool value);

	/**
	 * @return half-size when in orthographic mode.
	 */
	float GetOrthographicSize() const;
	void SetOrthographicSize(float value);

	void SetClearType(ClearType value);
	ClearType GetClearType() const;

	void SetRenderPath(RenderPath value);
	RenderPath GetRenderPath() const;

	void SetDepthTextureMode(DepthTextureMode value);
	DepthTextureMode GetDepthTextureMode() const;

	void SetClearColor(const Color& value);
	Color GetClearColor() const;

	void SetTargetTexture(RenderTexture value);
	RenderTexture GetTargetTexture();

	void SetAspect(float value);
	float GetAspect() const;

	void SetNearClipPlane(float value);
	float GetNearClipPlane() const;

	void SetFarClipPlane(float value);
	float GetFarClipPlane() const;
	
	void SetFieldOfView(float value);
	float GetFieldOfView() const;

	void SetRect(const Rect& value);
	const Rect& GetRect() const;

	const glm::mat4& GetProjectionMatrix();

	void GetVisibleGameObjects(std::vector<GameObject>& gameObjects);

	glm::vec3 WorldToScreenPoint(const glm::vec3& position);
	glm::vec3 ScreenToWorldPoint(const glm::vec3& position);

	void Render();
	void OnBeforeWorldDestroyed();
};

SUEDE_DEFINE_OBJECT_POINTER(Camera)

struct SUEDE_API CameraUtility {
	static void SetMain(Camera value);
	static Camera GetMain();

	static void OnPreRender();
	static void OnPostRender();
};
