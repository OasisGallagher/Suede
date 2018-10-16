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

SUEDE_DEFINE_OBJECT_POINTER(GameObject);

class SUEDE_API ICamera : public IComponent {
	SUEDE_DECLARE_COMPONENT()
	SUEDE_DECLARE_IMPL(Camera)

public:
	ICamera();

public:
	void SetDepth(int value);
	int GetDepth();

	bool GetPerspective() const;
	void SetPerspective(bool value);

	/**
	 * @return half-size when in orthographic mode.
	 */
	float GetOrthographicSize() const;
	void SetOrthographicSize(float value);

	void SetClearType(ClearType value);
	ClearType GetClearType();

	void SetRenderPath(RenderPath value);
	RenderPath GetRenderPath();

	void SetDepthTextureMode(DepthTextureMode value);
	DepthTextureMode GetDepthTextureMode();

	void SetClearColor(const Color& value);
	Color GetClearColor();

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

	Texture2D Capture();

	void Render();
	void OnBeforeWorldDestroyed();
};

SUEDE_DEFINE_CUSTOM_OBJECT_POINTER(Camera) {
	SUEDE_IMPLEMENT_CUSTOM_OBJECT_POINTER(Camera)

	static void main(Camera value);
	static Camera main();

	static void OnPreRender();
	static void OnPostRender();
};
