#pragma once
#include "object.h"
#include "bounds.h"
#include "transform.h"

SUEDE_DEFINE_OBJECT_POINTER(GameObject)

enum {
	RecalculateBoundsFlagsSelf = 1,
	RecalculateBoundsFlagsParent = 2,
	RecalculateBoundsFlagsChildren = 4,
	RecalculateBoundsFlagsAll = -1,
};

enum {
	GameObjectMessageMeshModified,
	GameObjectMessageLocalToWorldMatrixModified,

	GameObjectMessageUser = 1024,
};

class SUEDE_API IGameObject : public IObject {
	SUEDE_DEFINE_METATABLE_NAME(GameObject)
	SUEDE_DECLARE_IMPLEMENTATION(GameObject)

public:
	IGameObject();
	~IGameObject();

public:
	bool GetActive() const;

	void SetActiveSelf(bool value);
	bool GetActiveSelf() const;

	void SetLayer(uint value);
	uint GetLayer() const;

	int GetUpdateStrategy();

	void SendMessage(int messageID, void* parameter);

	const std::string& GetTag() const;
	bool SetTag(const std::string& value);

	void Update();
	void OnPreRender();
	void OnPostRender();
	void CullingUpdate();

	/**
	 * @brief shortcut for GetComponent<Transform>().
	 */
	Transform GetTransform();

	/**
	 * @returns bounds measured in the world space.
	 */
	const Bounds& GetBounds();
	void RecalculateBounds(int flags = RecalculateBoundsFlagsAll);

	void RecalculateUpdateStrategy();

public:	// Component system.
	Component AddComponent(const char* name);
	Component AddComponent(Component component);

	template <class T>
	typename std::enable_if<suede_is_intrusive_ptr<T>::value, T>::type AddComponent();

	template <class T>
	typename std::enable_if<!suede_is_intrusive_ptr<T>::value, intrusive_ptr<T>>::type AddComponent();

	Component GetComponent(suede_guid guid);
	Component GetComponent(const char* name);

	template <class T>
	typename std::enable_if<suede_is_intrusive_ptr<T>::value, T>::type GetComponent();

	template <class T>
	typename std::enable_if<!suede_is_intrusive_ptr<T>::value, intrusive_ptr<T>>::type GetComponent();

	template <class T>
	typename std::enable_if<suede_is_intrusive_ptr<T>::value, std::vector<T>>::type GetComponents();

	template <class T>
	typename std::enable_if<!suede_is_intrusive_ptr<T>::value, std::vector<intrusive_ptr<T>>>::type GetComponents();

	/**
	 * @param guid pass 0 to get all components.
	 */
	std::vector<Component> GetComponents(suede_guid guid);

	/**
	* @param name pass "" to get all components.
	*/
	std::vector<Component> GetComponents(const char* name);

private:
	Component AddComponent(suede_guid guid);
};

template <class T>
inline typename std::enable_if<suede_is_intrusive_ptr<T>::value, T>::type IGameObject::AddComponent() {
	return suede_dynamic_cast<T>(AddComponent(T::element_type::GetComponentGUID()));
}

template <class T>
inline typename std::enable_if<!suede_is_intrusive_ptr<T>::value, intrusive_ptr<T>>::type IGameObject::AddComponent() {
	return suede_dynamic_cast<intrusive_ptr<T>>(AddComponent(new T()));
}

template <class T>
inline typename std::enable_if<suede_is_intrusive_ptr<T>::value, T>::type IGameObject::GetComponent() {
	return suede_dynamic_cast<T>(GetComponent(T::element_type::GetComponentGUID()));
}

template <class T>
inline typename std::enable_if<!suede_is_intrusive_ptr<T>::value, intrusive_ptr<T>>::type IGameObject::GetComponent() {
	return suede_dynamic_cast<intrusive_ptr<T>>(GetComponent(T::GetComponentGUID()));
}

template <class T>
typename std::enable_if<suede_is_intrusive_ptr<T>::value, std::vector<T>>::type IGameObject::GetComponents() {
	std::vector<T> components;
	for (Component component : GetComponents(T::element_type::GetComponentGUID())) {
		components.push_back(suede_dynamic_cast<T>(component));
	}

	return components;
}

template <class T>
typename std::enable_if<!suede_is_intrusive_ptr<T>::value, std::vector<intrusive_ptr<T>>>::type IGameObject::GetComponents() {
	std::vector<intrusive_ptr<T>> components;
	for (Component component : GetComponents(T::GetComponentGUID())) {
		components.push_back(suede_dynamic_cast<intrusive_ptr<T>>(component));
	}

	return components;
}
