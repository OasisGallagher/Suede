#pragma once

#include "component.h"
#include "gameobject.h"
#include "internal/base/objectinternal.h"

class ComponentInternal : public ObjectInternal {
public:
	ComponentInternal(ObjectType type) : ObjectInternal(type), enabled_(true) {}

public:
	virtual bool GetEnabled() const { return enabled_; }
	virtual void SetEnabled(bool value) { enabled_ = value; }

	virtual void SetGameObject(GameObject go);
	virtual GameObject GetGameObject() { return gameObject_.lock(); }

	virtual Transform GetTransform() { return GetGameObject()->GetComponent<ITransform>(); }

	virtual void Awake() {}
	virtual void Update() {}
	virtual void CullingUpdate() {}

	virtual int GetUpdateStrategy() { return 0; }

protected:
	std::weak_ptr<GameObject::element_type> gameObject_;

private:
	bool enabled_;
};
