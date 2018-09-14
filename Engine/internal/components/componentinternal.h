#pragma once

#include "component.h"
#include "gameobject.h"
#include "internal/base/objectinternal.h"

class ComponentInternal : virtual public IComponent, public ObjectInternal {
public:
	ComponentInternal(ObjectType type) : ObjectInternal(type), enabled_(true) {}

public:
	virtual bool GetEnabled() const { return enabled_; }
	virtual void SetEnabled(bool value) { enabled_ = value; }

	virtual void SetGameObject(GameObject go);
	virtual GameObject GetGameObject() { return gameObject_.lock(); }

	virtual Transform GetTransform() { return GetGameObject()->GetComponent<ITransform>(); }

	virtual void CullingUpdate() {}
	virtual void RenderingUpdate() {}

protected:
	std::weak_ptr<GameObject::element_type> gameObject_;

private:
	bool enabled_;
};
