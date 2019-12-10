#pragma once
#include "mesh.h"
#include "texture.h"
#include "material.h"

struct Decal {
	Mesh mesh;
	Material material;
};

class SUEDE_API IProjector : public IComponent {
	SUEDE_DECLARE_COMPONENT()
	SUEDE_DEFINE_METATABLE_NAME(Projector)
	SUEDE_DECLARE_IMPLEMENTATION(Projector)

public:
	IProjector();

public:
	bool GetPerspective() const;
	void SetPerspective(bool value);

	/** 
	 * Half-size when in orthographic mode.
	 */
	float GetOrthographicSize() const;
	void SetOrthographicSize(float value);

	Texture GetTexture() const;
	void SetTexture(Texture value);

	void SetDepth(int value);
	int GetDepth() const;

	void SetAspect(float value);
	void SetNearClipPlane(float value);
	void SetFarClipPlane(float value);
	void SetFieldOfView(float value);

	float GetAspect() const;
	float GetNearClipPlane() const;
	float GetFarClipPlane() const;
	float GetFieldOfView() const;

	const Matrix4& GetProjectionMatrix();
};

SUEDE_DEFINE_OBJECT_POINTER(Projector)
