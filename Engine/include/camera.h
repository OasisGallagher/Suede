#pragma once
#include "rect.h"
#include "plane.h"
#include "color.h"
#include "texture.h"
#include "component.h"
#include "tools/enum.h"
#include "tools/event.h"
#include "math/matrix4.h"

BETTER_ENUM(ClearType, int,
	Color,
	Skybox
)

BETTER_ENUM(RenderPath, int,
	Forward,
	Deferred
)

BETTER_ENUM_MASK(DepthTextureMode, int,
	Depth = 1,
	DepthNormals = 2,
	MotionVector = 4
)

class SUEDE_API Camera : public Component {
	SUEDE_DECLARE_COMPONENT()
	SUEDE_DEFINE_METATABLE_NAME(Camera)
	SUEDE_DECLARE_IMPLEMENTATION(Camera)

public:
	Camera();
	~Camera();

public:
	void SetDepth(int vaue);
	int GetDepth() const;

	void SetCullingMask(int value);
	int GetCullingMask() const;

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

	void SetTargetTexture(RenderTexture* value);
	RenderTexture* GetTargetTexture();

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

	const Matrix4& GetProjectionMatrix();

	void GetVisibleGameObjects(std::vector<GameObject*>& gameObjects);

	Vector3 WorldToScreenPoint(const Vector3& position);
	Vector3 ScreenToWorldPoint(const Vector3& position);

	void Render();
	ref_ptr<Texture2D> Capture();

public:
	static void SetMain(Camera* value);
	static Camera* GetMain();

	static void OnPreRender();
	static void OnPostRender();
};
