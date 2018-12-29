#pragma once

#include "component.h"
#include "gameobject.h"
#include "internal/base/objectinternal.h"

// helper macro to define builtin suede components.
#define SUEDE_DEFINE_COMPONENT_INTERNAL(Class, ParentClass) \
	suede_guid I##Class::GetComponentGUID() { \
		static suede_guid guid = ClassNameToGUID(#Class); \
		return guid; \
	} \
	const char* I##Class::GetComponentName() { \
		return #Class; \
	} \
    bool I##Class::IsComponentType(suede_guid guid) const { \
		return guid == GetComponentGUID() || I##ParentClass::IsComponentType(guid); \
	} \
	bool I##Class::IsComponentType(const char* name) const { \
		return strcmp(name, GetComponentName()) == 0 || I##ParentClass::IsComponentType(name); \
	} \
	suede_guid I##Class::GetComponentInstanceGUID() const { \
		return I##Class::GetComponentGUID(); \
	} \
	const char* I##Class::GetComponentInstanceName() const { \
		return I##Class::GetComponentName(); \
	}

class ComponentInternal : public ObjectInternal {
public:
	ComponentInternal(ObjectType type) : ObjectInternal(type), enabled_(true) {}

public:
	virtual bool GetEnabled() const { return enabled_; }
	virtual void SetEnabled(bool value) { enabled_ = value; }

	virtual void SetGameObject(GameObject go);
	virtual GameObject GetGameObject() { return gameObject_; }

	virtual void OnMessage(int messageID, void* parameter) {}
	virtual Transform GetTransform() { return GetGameObject()->GetComponent<Transform>(); }

	virtual void Awake() {}
	virtual void Update() {}
	virtual void CullingUpdate() {}

	virtual int GetUpdateStrategy() { return 0; }

protected:
	IGameObject* gameObject_;

private:
	bool enabled_;
};
