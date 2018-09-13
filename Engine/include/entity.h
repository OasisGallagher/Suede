#pragma once
#include "mesh.h"
#include "bounds.h"
#include "renderer.h"
#include "animation.h"
#include "transform.h"
#include "particlesystem.h"

SUEDE_DEFINE_OBJECT_POINTER(Entity);
SUEDE_DECLARE_OBJECT_CREATER(Entity);

SUEDE_DEFINE_OBJECT_POINTER(Component);

enum {
	RecalculateBoundsFlagsSelf = 1,
	RecalculateBoundsFlagsParent = 2,
	RecalculateBoundsFlagsChildren = 4,
	RecalculateBoundsFlagsAll = -1,
};

#define SUEDE_ADD_COMPONENT(entity, T)	suede_dynamic_cast<T>(entity->AddComponent(ObjectType::T))
#define SUEDE_GET_COMPONENT(entity, T)	suede_dynamic_cast<T>(entity->GetComponent(ObjectType::T))

class SUEDE_API IEntity : virtual public IObject {
public:
	virtual bool GetActive() const = 0;

	virtual void SetActiveSelf(bool value) = 0;
	virtual bool GetActiveSelf() const = 0;

	virtual Component AddComponent(ObjectType type) = 0;
	virtual Component GetComponent(ObjectType type) = 0;

	virtual int GetUpdateStrategy() = 0;

	virtual const std::string& GetTag() const = 0;
	virtual bool SetTag(const std::string& value) = 0;

	virtual std::string GetName() const = 0;
	virtual void SetName(const std::string& value) = 0;

	virtual void CullingUpdate() = 0;
	virtual void RenderingUpdate() = 0;

	virtual Transform GetTransform() = 0;
	/**
	 * @returns bounds measured in the world space.
	 */
	virtual const Bounds& GetBounds() = 0;
	virtual void RecalculateBounds(int flags = RecalculateBoundsFlagsAll) = 0;

	virtual void RecalculateUpdateStrategy() = 0;
};
