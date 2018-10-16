#pragma once
#include "object.h"
#include "texture.h"

#define SUEDE_DECLARE_COMPONENT() \
public: \
    static suede_guid GetComponentGUID(); \
    bool IsComponentType(suede_guid classType) const;

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
	UpdateStrategyNone,
	UpdateStrategyCulling = 1,
	UpdateStrategyRendering = 1 << 1,
};

class SUEDE_API IComponent : public IObject {
	SUEDE_DECLARE_IMPL(Component)

public:
	void Awake();
	void OnRenderImage(RenderTexture src, RenderTexture dest, const Rect& normalizedRect);

	bool GetEnabled() const;
	void SetEnabled(bool value);

	void SetGameObject(GameObject value);
	GameObject GetGameObject();

	Transform GetTransform();

	void CullingUpdate();
	void RenderingUpdate();

	int GetUpdateStrategy();

public:
	static suede_guid ClassNameToGUID(const char* className);

public:
	static suede_guid GetComponentGUID() { return 0; }
	bool IsComponentType(suede_guid guid) const {
		return guid == GetComponentGUID();
	}

protected:
	IComponent(void* d);
};
