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

enum class WorldEventType {
	GameObjectCreated,
	GameObjectDestroyed,
	GameObjectTagChanged,
	GameObjectNameChanged,
	GameObjectParentChanged,
	GameObjectActiveChanged,
	GameObjectTransformChanged,
	GameObjectUpdateStrategyChanged,
	GameObjectComponentChanged,
	CameraDepthChanged,

	_Count,
};

#define DEFINE_WORLD_EVENT_PTR(type)	typedef std::shared_ptr<struct type> type ## Ptr

template <class Ptr, class... Args>
Ptr NewWorldEvent(Args... args) { return std::make_shared<Ptr::element_type>(args...); }

DEFINE_WORLD_EVENT_PTR(WorldEventBase);
struct WorldEventBase {
	virtual WorldEventType GetEventType() const = 0;
};

DEFINE_WORLD_EVENT_PTR(GameObjectEvent);

struct GameObjectEvent : public WorldEventBase {
	GameObject go;
};

template <class T>
struct ComponentEvent : public WorldEventBase {
	T component;
};

DEFINE_WORLD_EVENT_PTR(GameObjectEvent);

struct GameObjectCreatedEvent : public GameObjectEvent {
	virtual WorldEventType GetEventType() const { return WorldEventType::GameObjectCreated; }
};

DEFINE_WORLD_EVENT_PTR(GameObjectCreatedEvent);

struct GameObjectDestroyedEvent : public GameObjectEvent {
	virtual WorldEventType GetEventType() const { return WorldEventType::GameObjectDestroyed; }
};

DEFINE_WORLD_EVENT_PTR(GameObjectDestroyedEvent);

/**
 * @warning only gameObjects with non-null parant cound send this event.
 */
struct GameObjectParentChangedEvent : public GameObjectEvent {
	virtual WorldEventType GetEventType() const { return WorldEventType::GameObjectParentChanged; }
};

DEFINE_WORLD_EVENT_PTR(GameObjectParentChangedEvent);

/**
 * @warning only gameObjects with non-null parant cound send this event.
 */
struct GameObjectActiveChangedEvent : public GameObjectEvent {
	virtual WorldEventType GetEventType() const { return WorldEventType::GameObjectActiveChanged; }
};

DEFINE_WORLD_EVENT_PTR(GameObjectActiveChangedEvent);

/**
 * @warning only gameObjects with non-null parant cound send this event.
 */
struct GameObjectTagChangedEvent : public GameObjectEvent {
	virtual WorldEventType GetEventType() const { return WorldEventType::GameObjectTagChanged; }
};

DEFINE_WORLD_EVENT_PTR(GameObjectTagChangedEvent);

/**
 * @warning only gameObjects with non-null parant cound send this event.
 */
struct GameObjectNameChangedEvent : public GameObjectEvent {
	virtual WorldEventType GetEventType() const { return WorldEventType::GameObjectNameChanged; }
};

DEFINE_WORLD_EVENT_PTR(GameObjectNameChangedEvent);

struct GameObjectUpdateStrategyChangedEvent : public GameObjectEvent {
	virtual WorldEventType GetEventType() const { return WorldEventType::GameObjectUpdateStrategyChanged; }
};

DEFINE_WORLD_EVENT_PTR(GameObjectUpdateStrategyChangedEvent);

struct GameObjectComponentChangedEvent : public GameObjectEvent {
	virtual WorldEventType GetEventType() const { return WorldEventType::GameObjectComponentChanged; }

	bool added;
	Component component;
};

DEFINE_WORLD_EVENT_PTR(GameObjectComponentChangedEvent);

struct CameraDepthChangedEvent : ComponentEvent<Camera> {
	virtual WorldEventType GetEventType() const { return WorldEventType::CameraDepthChanged; }
};

DEFINE_WORLD_EVENT_PTR(CameraDepthChangedEvent);

DEFINE_WORLD_EVENT_PTR(GameObjectTransformChangedEvent);
struct GameObjectTransformChangedEvent : public GameObjectEvent {
	virtual WorldEventType GetEventType() const { return WorldEventType::GameObjectTransformChanged; }

	// Hw: local or world(0).
	// Lw: position rotation or scale.
	uint prs;
};

#undef DEFINE_WORLD_EVENT_POINTER

class WorldEventListener {
public:
	virtual void OnWorldEvent(WorldEventBasePtr e) = 0;
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

class SUEDE_API World : public Singleton2<World> {
	friend class Singleton<World>;
	SUEDE_DECLARE_IMPLEMENTATION(World)

public:
	void Initialize();
	void CullingUpdate();
	void RenderingUpdate();
	void Finalize();

	Object CreateObject(ObjectType type);

	void DestroyGameObject(uint id);
	void DestroyGameObject(GameObject go);

	GameObject Import(const std::string& path, GameObjectImportedListener* listener);
	GameObject Import(const std::string& path, Lua::Func<void, GameObject, const std::string&> callback);

	bool ImportTo(GameObject go, const std::string& path, GameObjectImportedListener* listener);

	Transform GetRootTransform();

	GameObject GetGameObject(uint id);
	void WalkGameObjectHierarchy(WorldGameObjectWalker* walker);

	void FireEvent(WorldEventBasePtr e);
	void FireEventImmediate(WorldEventBasePtr e);
	void AddEventListener(WorldEventListener* listener);
	void RemoveEventListener(WorldEventListener* listener);

	void GetDecals(std::vector<Decal>& container);

public:
	template <class T> std::vector<std::shared_ptr<T>> GetComponents();
	std::vector<GameObject> GetGameObjectsOfComponent(suede_guid guid);

private:
	World();
	~World();
};

template <class T>
std::vector<std::shared_ptr<T>> World::GetComponents() {
	std::vector<std::shared_ptr<T>> components;
	for (GameObject go : GetGameObjectsOfComponent(T::GetComponentGUID())) {
		components.push_back(go->GetComponent<T>());
	}

	return components;
}
