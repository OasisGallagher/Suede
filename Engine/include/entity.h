#pragma once
#include "mesh.h"
#include "renderer.h"
#include "animation.h"
#include "transform.h"

SUEDE_DEFINE_OBJECT_POINTER(Entity);
SUEDE_DECLARE_OBJECT_CREATER(Entity);

class SUEDE_API IEntity : virtual public IObject {
public:
	virtual bool GetActive() const = 0;

	virtual void SetActiveSelf(bool value) = 0;
	virtual bool GetActiveSelf() const = 0;

	virtual const std::string& GetTag() const = 0;
	virtual bool SetTag(const std::string& value) = 0;

	virtual std::string GetName() const = 0;
	virtual void SetName(const std::string& value) = 0;

	virtual void Update() = 0;

	virtual void SetTransform(Transform value) = 0;
	virtual Transform GetTransform() const = 0;

	virtual void SetAnimation(Animation value) = 0;
	virtual Animation GetAnimation() = 0;

	virtual void SetMesh(Mesh value) = 0;
	virtual Mesh GetMesh() = 0;

	virtual void SetRenderer(Renderer value) = 0;
	virtual Renderer GetRenderer() = 0;
};