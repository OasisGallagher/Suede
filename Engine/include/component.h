#pragma once
#include "object.h"
#include "texture.h"

#define SUEDE_DECLARE_COMPONENT() \
public: \
    static suede_guid GetComponentGUID(); \
	static const char* GetComponentName(); \
    bool IsComponentType(suede_guid guid) const; \
    bool IsComponentType(const std::string& name) const; \
	suede_guid GetComponentInstanceGUID() const;

#define SUEDE_DEFINE_COMPONENT(Class, ParentClass) \
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
	bool Class::IsComponentType(const std::string& name) const { \
		return name == GetComponentName() || ParentClass::IsComponentType(name); \
	} \
	suede_guid Class::GetComponentInstanceGUID() const { return Class::GetComponentGUID(); }

SUEDE_DEFINE_OBJECT_POINTER(GameObject);
SUEDE_DEFINE_OBJECT_POINTER(Transform);

enum {
	UpdateStrategyNone,
	UpdateStrategyCulling = 1,
	UpdateStrategyRendering = 1 << 1,
};

class SUEDE_API IComponent : public IObject {
	SUEDE_DECLARE_IMPLEMENTATION(Component)

public:
	virtual void Awake();
	virtual void OnRenderImage(RenderTexture src, RenderTexture dest, const Rect& normalizedRect);

public:
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
	static const char* GetComponentName() { return ""; }

public:
	virtual bool IsComponentType(suede_guid guid) const {
		return guid == GetComponentGUID();
	}

	virtual bool IsComponentType(const std::string& name) const {
		return name == GetComponentName();
	}

	virtual suede_guid GetComponentInstanceGUID() const {
		return IComponent::GetComponentGUID();
	}

protected:
	IComponent(void* d);
};

SUEDE_DEFINE_CUSTOM_OBJECT_POINTER(Component) {
	SUEDE_IMPLEMENT_CUSTOM_OBJECT_POINTER(Component)

	static bool Register(suede_guid guid, Object(*)());
	static bool Register(const std::string& name, Object(*)());
};
