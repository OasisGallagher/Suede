#pragma once

#include "component.h"
#include "gameobject.h"
#include "internal/base/objectinternal.h"

/**
 * Helper macro to define builtin suede components.
 */
#define SUEDE_DEFINE_COMPONENT_INTERNAL(Class, ParentClass) \
	suede_guid Class::GetComponentGUID() { \
		static suede_guid guid = ClassNameToGUID(#Class); \
		return guid; \
	} \
	const char* Class::GetComponentName() { \
		return #Class; \
	} \
    bool Class::IsComponentType(suede_guid guid) const { \
		return guid == GetComponentGUID() || ParentClass::IsComponentType(guid); \
	} \
	bool Class::IsComponentType(const char* name) const { \
		return strcmp(name, GetComponentName()) == 0 || ParentClass::IsComponentType(name); \
	} \
	suede_guid Class::GetComponentInstanceGUID() const { \
		return Class::GetComponentGUID(); \
	} \
	const char* Class::GetComponentInstanceName() const { \
		return Class::GetComponentName(); \
	}

class RenderingContext;
class ComponentInternal : public ObjectInternal {
public:
	ComponentInternal(ObjectType type);

public:
	bool GetEnabled() const { return enabled_; }
	void SetEnabled(bool value) { enabled_ = value; }

	bool GetActiveAndEnabled() const { return gameObject_->GetActive() && enabled_; }

	void SetGameObject(GameObject* go);
	GameObject* GetGameObject() { return gameObject_; }

	Transform* GetTransform() { return GetGameObject()->GetComponent<Transform>(); }

	virtual void Awake() {}
	virtual void Update(float deltaTime) {}
	virtual void CullingUpdate(float deltaTime) {}

	virtual int GetUpdateStrategy() { return 0; }
	virtual void OnMessage(int messageID, void* parameter) {}

protected:
	GameObject* gameObject_ = nullptr;
	RenderingContext* context_ = nullptr;

private:
	bool enabled_ = true;
};
