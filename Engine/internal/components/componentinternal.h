#pragma once

#include "entity.h"
#include "component.h"
#include "internal/base/objectinternal.h"

class ComponentInternal : virtual public IComponent, public ObjectInternal {
public:
	ComponentInternal(ObjectType type) : ObjectInternal(type) {}

public:
	virtual void SetEntity(Entity entity);
	virtual Entity GetEntity() { return entity_.lock(); }

	virtual void Update() {}

protected:
	std::weak_ptr<Entity::element_type> entity_;
};
