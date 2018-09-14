#pragma once

#include "entity.h"
#include "component.h"
#include "internal/base/objectinternal.h"

class ComponentInternal : virtual public IComponent, public ObjectInternal {
public:
	ComponentInternal(ObjectType type) : ObjectInternal(type), enabled_(true) {}

public:
	virtual bool GetEnabled() const { return enabled_; }
	virtual void SetEnabled(bool value) { enabled_ = value; }

	virtual void SetEntity(Entity entity);
	virtual Entity GetEntity() { return entity_.lock(); }

	virtual Transform GetTransform() { return GetEntity()->GetComponent<ITransform>(); }

	virtual void CullingUpdate() {}
	virtual void RenderingUpdate() {}

protected:
	std::weak_ptr<Entity::element_type> entity_;

private:
	bool enabled_;
};
