#pragma once
#include "object.h"

#define SUPER_CLASS_DECLARATION() \
public: \
	static suede_guid GetTypeID() { return 0; } \
	virtual bool IsClassType(suede_guid classType) const { \
		return classType == GetTypeID(); \
	}

// This macro must be included in the declaration of any subclass of Component.
// It declares variables used in type checking.
#define RTTI_CLASS_DECLARATION(classname, parentclass) \
public: \
    static suede_guid GetTypeID() { \
		static suede_guid type = ComponentGUID(#classname); \
		return type; \
	} \
    virtual bool IsClassType(suede_guid classType) const { \
		return classType == classname::GetTypeID() || parentclass::IsClassType(classType); \
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
	SUPER_CLASS_DECLARATION()

public:
	virtual bool GetEnabled() const = 0;
	virtual void SetEnabled(bool value) = 0;

	virtual void SetGameObject(GameObject go) = 0;
	virtual GameObject GetGameObject() = 0;

	virtual Transform GetTransform() = 0;

	virtual void CullingUpdate() = 0;
	virtual void RenderingUpdate() = 0;

	virtual int GetUpdateStrategy() = 0;

public:
	static suede_guid ComponentGUID(const char* classname);
};
