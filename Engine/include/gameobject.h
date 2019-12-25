#pragma once
#include "object.h"
#include "bounds.h"
#include "transform.h"
#include "mainmtevent.h"

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

enum class ComponentEventType {
	Added,
	Removed,
	Modified,
};

#define DEFAULT_GAME_OBJECT_NAME "New GameObject"

class SUEDE_API GameObject : public Object {
	SUEDE_DEFINE_METATABLE_NAME(GameObject)
	SUEDE_DECLARE_IMPLEMENTATION(GameObject)

public:
	GameObject(const char* name = DEFAULT_GAME_OBJECT_NAME);
	~GameObject() {}

public:
	static main_mt_event<ref_ptr<GameObject>> created;
	static main_mt_event<ref_ptr<GameObject>> destroyed;
	static main_mt_event<ref_ptr<GameObject>> tagChanged;
	static main_mt_event<ref_ptr<GameObject>> nameChanged;
	static main_mt_event<ref_ptr<GameObject>> parentChanged;
	static main_mt_event<ref_ptr<GameObject>> activeChanged;
	static main_mt_event<ref_ptr<GameObject>> updateStrategyChanged;
	static main_mt_event<ref_ptr<GameObject>, int> transformChanged;
	static main_mt_event<ref_ptr<GameObject>, ComponentEventType, ref_ptr<Component>> componentChanged;

public:
	bool GetActive() const;

	void SetActiveSelf(bool value);
	bool GetActiveSelf() const;

	int GetUpdateStrategy();

	void SendMessage(int messageID, void* parameter);

	const std::string& GetTag() const;
	bool SetTag(const std::string& value);

	void Update();
	void CullingUpdate();

	/**
	 * @brief shortcut for GetComponent<Transform>().
	 */
	Transform* GetTransform();

	/**
	 * @returns bounds measured in the world space.
	 */
	const Bounds& GetBounds();
	void RecalculateBounds(int flags = RecalculateBoundsFlagsAll);

	void RecalculateUpdateStrategy();

public:	// Component system.
	Component* AddComponent(const char* name);
	Component* AddComponent(Component* component);

	template <class T> T* AddComponent();

	Component* GetComponent(suede_guid guid);
	Component* GetComponent(const char* name);

	template <class T> T* GetComponent();
	template <class T> std::vector<T*> GetComponents();

	/**
	 * @param guid pass 0 to get all components.
	 */
	std::vector<Component*> GetComponents(suede_guid guid);

	/**
	* @param name pass "" to get all components.
	*/
	std::vector<Component*> GetComponents(const char* name);

private:
	Component* AddComponent(suede_guid guid);
};

template <class T> T* GameObject::AddComponent() {
	return dynamic_cast<T*>(AddComponent(new T()));
}

template <class T> T* GameObject::GetComponent() {
	return dynamic_cast<T*>(GetComponent(T::GetComponentGUID()));
}

template <class T> std::vector<T*> GameObject::GetComponents() {
	std::vector<T*> components;
	for (Component* component : GetComponents(T::GetComponentGUID())) {
		components.push_back(dynamic_cast<T*>(component));
	}

	return components;
}
