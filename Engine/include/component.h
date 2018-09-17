#pragma once
#include "object.h"

#define SUEDE_DECLARE_COMPONENT() \
public: \
    static suede_guid GetComponentGUID(); \
    virtual bool IsComponentType(suede_guid classType) const;

#define SUEDE_DEFINE_COMPONENT(Class, ParentClass) \
	suede_guid Class::GetComponentGUID() { \
		static suede_guid guid = ClassNameToGUID(#Class); \
		return guid; \
	} \
    bool Class::IsComponentType(suede_guid classType) const { \
		return classType == GetComponentGUID() || ParentClass::IsComponentType(classType); \
	}

SUEDE_DEFINE_OBJECT_POINTER(GameObject);
SUEDE_DEFINE_OBJECT_POINTER(Transform);
SUEDE_DEFINE_OBJECT_POINTER(Component);

enum {
	UpdateStrategyNone = 0,
	UpdateStrategyCulling = 1,
	UpdateStrategyRendering = 1 << 1,
};

class SUEDE_API IComponent : virtual public IObject {
public:
	virtual bool GetEnabled() const = 0;
	virtual void SetEnabled(bool value) = 0;

	virtual void SetGameObject(GameObject value) = 0;
	virtual GameObject GetGameObject() = 0;

	virtual Transform GetTransform() = 0;

	virtual void CullingUpdate() = 0;
	virtual void RenderingUpdate() = 0;

	virtual int GetUpdateStrategy() = 0;

public:
	static suede_guid ClassNameToGUID(const char* className);

public:
	static suede_guid GetComponentGUID() { return 0; }
	virtual bool IsComponentType(suede_guid guid) const { return guid == GetComponentGUID(); }
};
