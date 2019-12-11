#pragma once
#include "mesh.h"
#include "texture.h"
#include "material.h"

struct Decal {
	ref_ptr<Mesh> mesh;
	ref_ptr<Material> material;
};

class SUEDE_API Projector : public Component {
	SUEDE_DECLARE_COMPONENT()
	SUEDE_DEFINE_METATABLE_NAME(Projector)
	SUEDE_DECLARE_IMPLEMENTATION(Projector)

public:
	Projector();

public:
	bool GetPerspective() const;
	void SetPerspective(bool value);

	/** 
	 * Half-size when in orthographic mode.
	 */
	float GetOrthographicSize() const;
	void SetOrthographicSize(float value);

	Texture* GetTexture() const;
	void SetTexture(Texture* value);

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
