#pragma once
#include <vector>

#include "lua++.h"

#include "object.h"
#include "camera.h"
#include "transform.h"
#include "gameobject.h"
#include "environment.h"
#include "tools/singleton.h"
#include "gameobjectimportedlistener.h"

BETTER_ENUM(WorldEventType, int,
	GameObjectCreated,
	GameObjectDestroyed,
	GameObjectTagChanged,
	GameObjectNameChanged,
	GameObjectParentChanged,
	GameObjectActiveChanged,
	GameObjectTransformChanged,
	GameObjectUpdateStrategyChanged,
	GameObjectComponentChanged,
	CameraDepthChanged
)

struct WorldEventBase : public intrusive_ref_counter<> {
	virtual WorldEventType GetEventType() const = 0;
};

struct GameObjectEvent : public WorldEventBase {
	GameObject go;
	GameObjectEvent(GameObject go) : go(go) {}
};

template <class T>
struct ComponentEvent : public WorldEventBase {
	T component;
	ComponentEvent(T component) : component(component) {}
};

struct GameObjectCreatedEvent : GameObjectEvent {
	GameObjectCreatedEvent(GameObject go) : GameObjectEvent(go) {}
	virtual WorldEventType GetEventType() const { return WorldEventType::GameObjectCreated; }
};

struct GameObjectDestroyedEvent : public GameObjectEvent {
	GameObjectDestroyedEvent(GameObject go) : GameObjectEvent(go) {}
	virtual WorldEventType GetEventType() const { return WorldEventType::GameObjectDestroyed; }
};

/**
 * @warning only gameObjects with non-null parant could send this event.
 */
struct GameObjectParentChangedEvent : public GameObjectEvent {
	GameObjectParentChangedEvent(GameObject go) : GameObjectEvent(go) {}
	virtual WorldEventType GetEventType() const { return WorldEventType::GameObjectParentChanged; }
};

/**
 * @warning only gameObjects with non-null parant cound send this event.
 */
struct GameObjectActiveChangedEvent : public GameObjectEvent {
	GameObjectActiveChangedEvent(GameObject go) : GameObjectEvent(go) {}
	virtual WorldEventType GetEventType() const { return WorldEventType::GameObjectActiveChanged; }
};

/**
 * @warning only gameObjects with non-null parant cound send this event.
 */
struct GameObjectTagChangedEvent : public GameObjectEvent {
	GameObjectTagChangedEvent(GameObject go) : GameObjectEvent(go) {}
	virtual WorldEventType GetEventType() const { return WorldEventType::GameObjectTagChanged; }
};

/**
 * @warning only gameObjects with non-null parant cound send this event.
 */
struct GameObjectNameChangedEvent : public GameObjectEvent {
	GameObjectNameChangedEvent(GameObject go) : GameObjectEvent(go) {}
	virtual WorldEventType GetEventType() const { return WorldEventType::GameObjectNameChanged; }
};

struct GameObjectUpdateStrategyChangedEvent : public GameObjectEvent {
	GameObjectUpdateStrategyChangedEvent(GameObject go) : GameObjectEvent(go) {}
	virtual WorldEventType GetEventType() const { return WorldEventType::GameObjectUpdateStrategyChanged; }
};

struct GameObjectTransformChangedEvent : public GameObjectEvent {
	GameObjectTransformChangedEvent(GameObject go, uint prs) : GameObjectEvent(go), prs(prs) {}
	virtual WorldEventType GetEventType() const { return WorldEventType::GameObjectTransformChanged; }

	// Hw: local or world(0).
	// Lw: position rotation or scale.
	uint prs;
};

struct GameObjectComponentChangedEvent : public GameObjectEvent {
	GameObjectComponentChangedEvent(GameObject go, int state, Component component) :GameObjectEvent(go), state(state), component(component) {}
	virtual WorldEventType GetEventType() const { return WorldEventType::GameObjectComponentChanged; }

	enum {
		ComponentAdded,
		ComponentRemoved,
		ComponentModified,
	};

	int state;
	Component component;
};

struct CameraDepthChangedEvent : ComponentEvent<Camera> {
	CameraDepthChangedEvent(Camera camera) : ComponentEvent(camera) {}
	virtual WorldEventType GetEventType() const { return WorldEventType::CameraDepthChanged; }
};

class WorldEventListener {
public:
	virtual void OnWorldEvent(WorldEventBase* e) = 0;
};

enum class WalkCommand {
	Next,
	Break,
	Continue,
};

class WorldGameObjectWalker {
public:
	virtual WalkCommand OnWalkGameObject(GameObject go) = 0;
};

struct Decal;

class SUEDE_API World : private singleton2<World> {
	friend class singleton<World>;
	SUEDE_DECLARE_IMPLEMENTATION(World)

public:
	static void Initialize();
	static void Finalize();

	static void Update();
	static void CullingUpdate();

	static void DestroyGameObject(uint id);
	static void DestroyGameObject(GameObject go);

	static GameObject Import(const std::string& path, GameObjectImportedListener* listener);
	static GameObject Import(const std::string& path, Lua::Func<void, GameObject, const std::string&> callback);

	static bool ImportTo(GameObject go, const std::string& path, GameObjectImportedListener* listener);

	static Transform GetRootTransform();

	static GameObject GetGameObject(uint id);

	static void WalkGameObjectHierarchy(WorldGameObjectWalker* walker);

	static void FireEvent(WorldEventBase* e);
	static void FireEventImmediate(WorldEventBase& e);
	static void AddEventListener(WorldEventListener* listener);
	static void RemoveEventListener(WorldEventListener* listener);

	static void GetDecals(std::vector<Decal>& container);

public:
	template <class T>
	static typename std::enable_if<suede_is_intrusive_ptr<T>::value, std::vector<T>>::type GetComponents();

	template <class T>
	static typename std::enable_if<!suede_is_intrusive_ptr<T>::value, std::vector<intrusive_ptr<T>>>::type GetComponents();

	static std::vector<GameObject> GetGameObjectsOfComponent(suede_guid guid);

private:
	World();
};

template <class T>
typename std::enable_if<suede_is_intrusive_ptr<T>::value, std::vector<T>>::type World::GetComponents() {
	std::vector<T> components;
	for (GameObject go : GetGameObjectsOfComponent(T::element_type::GetComponentGUID())) {
		components.push_back(go->GetComponent<T>());
	}

	return components;
}

template <class T>
typename std::enable_if<!suede_is_intrusive_ptr<T>::value, std::vector<intrusive_ptr<T>>>::type World::GetComponents() {
	std::vector<intrusive_ptr<T>> components;
	for (GameObject go : GetGameObjectsOfComponent(T::GetComponentGUID())) {
		components.push_back(go->GetComponent<T>());
	}

	return components;
}
