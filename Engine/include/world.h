#pragma once
#include <vector>
#include "object.h"
#include "camera.h"
#include "transform.h"
#include "gameobject.h"
#include "environment.h"
#include "tools/singleton.h"
#include "gameobjectloadedlistener.h"

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

#define DEFINE_WORLD_EVENT_POINTER(type)	typedef std::shared_ptr<struct type> type ## Pointer

template <class Ptr, class... Args>
Ptr NewWorldEvent(Args... args) { return std::make_shared<Ptr::element_type>(args...); }

DEFINE_WORLD_EVENT_POINTER(WorldEventBase);
struct WorldEventBase {
	virtual WorldEventType GetEventType() const = 0;
};

DEFINE_WORLD_EVENT_POINTER(GameObjectEvent);

struct GameObjectEvent : public WorldEventBase {
	GameObject go;
};

template <class T>
struct ComponentEvent : public WorldEventBase {
	T component;
};

DEFINE_WORLD_EVENT_POINTER(GameObjectEvent);

struct GameObjectCreatedEvent : public GameObjectEvent {
	virtual WorldEventType GetEventType() const { return WorldEventType::GameObjectCreated; }
};

DEFINE_WORLD_EVENT_POINTER(GameObjectCreatedEvent);

struct GameObjectDestroyedEvent : public GameObjectEvent {
	virtual WorldEventType GetEventType() const { return WorldEventType::GameObjectDestroyed; }
};

DEFINE_WORLD_EVENT_POINTER(GameObjectDestroyedEvent);

/**
 * @warning only gameObjects with non-null parant cound send this event.
 */
struct GameObjectParentChangedEvent : public GameObjectEvent {
	virtual WorldEventType GetEventType() const { return WorldEventType::GameObjectParentChanged; }
};

DEFINE_WORLD_EVENT_POINTER(GameObjectParentChangedEvent);

/**
 * @warning only gameObjects with non-null parant cound send this event.
 */
struct GameObjectActiveChangedEvent : public GameObjectEvent {
	virtual WorldEventType GetEventType() const { return WorldEventType::GameObjectActiveChanged; }
};

DEFINE_WORLD_EVENT_POINTER(GameObjectActiveChangedEvent);

/**
 * @warning only gameObjects with non-null parant cound send this event.
 */
struct GameObjectTagChangedEvent : public GameObjectEvent {
	virtual WorldEventType GetEventType() const { return WorldEventType::GameObjectTagChanged; }
};

DEFINE_WORLD_EVENT_POINTER(GameObjectTagChangedEvent);

/**
 * @warning only gameObjects with non-null parant cound send this event.
 */
struct GameObjectNameChangedEvent : public GameObjectEvent {
	virtual WorldEventType GetEventType() const { return WorldEventType::GameObjectNameChanged; }
};

DEFINE_WORLD_EVENT_POINTER(GameObjectNameChangedEvent);

struct GameObjectUpdateStrategyChangedEvent : public GameObjectEvent {
	virtual WorldEventType GetEventType() const { return WorldEventType::GameObjectUpdateStrategyChanged; }
};

DEFINE_WORLD_EVENT_POINTER(GameObjectUpdateStrategyChangedEvent);

struct GameObjectComponentChangedEvent : public GameObjectEvent {
	virtual WorldEventType GetEventType() const { return WorldEventType::GameObjectComponentChanged; }

	bool added;
	Component component;
};

DEFINE_WORLD_EVENT_POINTER(GameObjectComponentChangedEvent);

struct CameraDepthChangedEvent : ComponentEvent<Camera> {
	virtual WorldEventType GetEventType() const { return WorldEventType::CameraDepthChanged; }
};

DEFINE_WORLD_EVENT_POINTER(CameraDepthChangedEvent);

DEFINE_WORLD_EVENT_POINTER(GameObjectTransformChangedEvent);
struct GameObjectTransformChangedEvent : public GameObjectEvent {
	virtual WorldEventType GetEventType() const { return WorldEventType::GameObjectTransformChanged; }

	// Hw: local or world(0).
	// Lw: position rotation or scale.
	uint prs;
};

#undef DEFINE_WORLD_EVENT_POINTER

class WorldEventListener {
public:
	virtual void OnWorldEvent(WorldEventBasePointer e) = 0;
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
public:
	virtual void Initialize() = 0;
	virtual void CullingUpdate() = 0;
	virtual void RenderingUpdate() = 0;
	virtual void Finalize() = 0;

	virtual Object CreateObject(ObjectType type) = 0;

	virtual void DestroyGameObject(uint id) = 0;
	virtual void DestroyGameObject(GameObject go) = 0;

	virtual GameObject Import(const std::string& path, GameObjectLoadedListener* listener) = 0;
	virtual bool ImportTo(GameObject go, const std::string& path, GameObjectLoadedListener* listener) = 0;

	virtual Transform GetRootTransform() = 0;

	virtual GameObject GetGameObject(uint id) = 0;
	virtual void WalkGameObjectHierarchy(WorldGameObjectWalker* walker) = 0;

	virtual void FireEvent(WorldEventBasePointer e) = 0;
	virtual void FireEventImmediate(WorldEventBasePointer e) = 0;
	virtual void AddEventListener(WorldEventListener* listener) = 0;
	virtual void RemoveEventListener(WorldEventListener* listener) = 0;

	virtual void GetDecals(std::vector<Decal>& container) = 0;

public:
	template <class T> std::vector<std::shared_ptr<T>> GetComponents();
	virtual std::vector<GameObject> GetGameObjectsOfComponent(suede_guid guid) = 0;
};

template <class T>
std::vector<std::shared_ptr<T>> World::GetComponents() {
	std::vector<std::shared_ptr<T>> components;
	for (GameObject go : GetGameObjectsOfComponent(T::GetComponentGUID())) {
		components.push_back(go->GetComponent<T>());
	}

	return components;
}
