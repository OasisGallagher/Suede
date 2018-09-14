#pragma once
#include "object.h"

#define CLASS_ID(classname)	std::hash<std::string>()(#classname)

#define SUPER_CLASS_DECLARATION() \
public: \
	static suede_typeid GetTypeID() { return 0; } \
	virtual bool IsClassType(suede_typeid classType) const { \
		return classType == GetTypeID(); \
	}

// This macro must be included in the declaration of any subclass of Component.
// It declares variables used in type checking.
#define RTTI_CLASS_DECLARATION(classname, parentclass) \
public: \
    static suede_typeid GetTypeID() { \
		static suede_typeid type = CLASS_ID(classname); \
		return type; \
	} \
    virtual bool IsClassType(suede_typeid classType) const { \
		return classType == classname::GetTypeID() || parentclass::IsClassType(classType); \
	}

SUEDE_DEFINE_OBJECT_POINTER(Entity);
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

	virtual void SetEntity(Entity entity) = 0;
	virtual Entity GetEntity() = 0;

	virtual Transform GetTransform() = 0;

	virtual void CullingUpdate() = 0;
	virtual void RenderingUpdate() = 0;

	virtual int GetUpdateStrategy() = 0;
};
