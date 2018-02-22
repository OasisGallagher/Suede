#pragma once
#include "entity.h"
#include "texture.h"

struct Decal {
	Texture texture;
	glm::mat4 matrix;

	MeshTopology topology;
	std::vector<uint> indexes;
	std::vector<glm::vec3> positions;
};

class SUEDE_API IProjector : virtual public IEntity {
public:
	virtual bool GetPerspective() const = 0;
	virtual void SetPerspective(bool value) = 0;

	/** 
	 * Half-size when in orthographic mode.
	 */
	virtual float GetOrthographicSize() const = 0;
	virtual void SetOrthographicSize(float value) = 0;

	virtual Texture GetTexture() const = 0;
	virtual void SetTexture(Texture value) = 0;

	virtual void SetDepth(int value) = 0;
	virtual int GetDepth() const = 0;

	virtual void SetAspect(float value) = 0;
	virtual void SetNearClipPlane(float value) = 0;
	virtual void SetFarClipPlane(float value) = 0;
	virtual void SetFieldOfView(float value) = 0;

	virtual float GetAspect() const = 0;
	virtual float GetNearClipPlane() const = 0;
	virtual float GetFarClipPlane() const = 0;
	virtual float GetFieldOfView() const = 0;

	virtual const glm::mat4& GetProjectionMatrix() const = 0;
};

SUEDE_DEFINE_OBJECT_POINTER(Projector);
SUEDE_DECLARE_OBJECT_CREATER(Projector);
