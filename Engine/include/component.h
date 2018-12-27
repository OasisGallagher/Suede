#pragma once
#include "object.h"
#include "texture.h"

#define SUEDE_DECLARE_COMPONENT() \
public: \
    static suede_guid GetComponentGUID(); \
	static const char* GetComponentName(); \
    bool IsComponentType(suede_guid guid) const; \
    bool IsComponentType(const char* name) const; \
	suede_guid GetComponentInstanceGUID() const; \
	const char* GetComponentInstanceName() const;

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
	bool Class::IsComponentType(const char* name) const { \
		return strcmp(name, GetComponentName()) == 0 || ParentClass::IsComponentType(name); \
	} \
	suede_guid Class::GetComponentInstanceGUID() const { \
		return Class::GetComponentGUID(); \
	} \
	const char* Class::GetComponentInstanceName() const { \
		return Class::GetComponentName(); \
	}

SUEDE_DEFINE_OBJECT_POINTER(GameObject)
SUEDE_DEFINE_OBJECT_POINTER(Transform)

enum {
	UpdateStrategyNone,
	UpdateStrategyCulling = 1,
	UpdateStrategyRendering = 1 << 1,
};

class SUEDE_API IComponent : public IObject {
	SUEDE_DEFINE_METATABLE_NAME(Component)
	SUEDE_DECLARE_IMPLEMENTATION(Component)

public:
	virtual void Awake();
	virtual void Update();
	virtual void OnRenderImage(RenderTexture src, RenderTexture dest, const Rect& normalizedRect);

public:
	bool GetEnabled() const;
	void SetEnabled(bool value);

	void SetGameObject(GameObject value);
	GameObject GetGameObject();

	Transform GetTransform();
	void OnMessage(int messageID, void* parameter);

	void CullingUpdate();

	int GetUpdateStrategy();

public:
	static suede_guid GetComponentGUID() { return 0; }
	static const char* GetComponentName() { return ""; }
	static suede_guid ClassNameToGUID(const char* className);

public:
	/**
	 * @brief whether components of same type (or subtype) could be added more than once to a GameObject.
	 */
	virtual bool AllowMultiple() const { return false; }

	virtual bool IsComponentType(suede_guid guid) const { return guid == GetComponentGUID(); }
	virtual bool IsComponentType(const char* name) const { return strcmp(name, GetComponentName()) == 0; }

	/**
	 * @brief polymorphism version to get component GUID.
	 */
	virtual suede_guid GetComponentInstanceGUID() const { return GetComponentGUID(); }

	/**
	* @brief polymorphism version to get component name.
	*/
	virtual const char* GetComponentInstanceName() const { return GetComponentName(); }

protected:
	IComponent(void* d);
};

SUEDE_DEFINE_OBJECT_POINTER(Component)

struct SUEDE_API ComponentUtility {
	static bool Register(suede_guid guid, Object(*)());
	static bool Register(const char* name, Object(*)());
};
