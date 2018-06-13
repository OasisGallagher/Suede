#pragma once
#include <vector>
#include "entity.h"
#include "object.h"
#include "camera.h"
#include "transform.h"
#include "environment.h"
#include "entityloadedlistener.h"

enum WorldEventType {
	WorldEventTypeEntityCreated,
	WorldEventTypeEntityDestroyed,
	WorldEventTypeEntityTagChanged,
	WorldEventTypeEntityNameChanged,
	WorldEventTypeEntityParentChanged,
	WorldEventTypeEntityTransformChanged,
	WorldEventTypeEntityActiveChanged,

	WorldEventTypeCameraDepthChanged,

	WorldEventTypeCount,
};

#define DEFINE_WORLD_EVENT_POINTER(type)	typedef std::shared_ptr<struct type> type ## Pointer

template <class Ptr>
Ptr NewWorldEvent() { return std::make_shared<Ptr::element_type>(); }

DEFINE_WORLD_EVENT_POINTER(WorldEventBase);
struct WorldEventBase {
	virtual WorldEventType GetEventType() const = 0;
	bool operator < (WorldEventBasePointer other) const {
		return Compare(other);
	}

	virtual bool Compare(WorldEventBasePointer other) const = 0;
};

DEFINE_WORLD_EVENT_POINTER(EntityEvent);

struct EntityEvent : public WorldEventBase {
	Entity entity;

	virtual bool Compare(WorldEventBasePointer other) const {
		return entity->GetInstanceID() < suede_static_cast<EntityEventPointer>(other)->entity->GetInstanceID();
	}
};

DEFINE_WORLD_EVENT_POINTER(EntityEvent);

struct EntityCreatedEvent : public EntityEvent {
	virtual WorldEventType GetEventType() const { return WorldEventTypeEntityCreated; }
};

DEFINE_WORLD_EVENT_POINTER(EntityCreatedEvent);

struct EntityDestroyedEvent : public EntityEvent {
	virtual WorldEventType GetEventType() const { return WorldEventTypeEntityDestroyed; }
};

DEFINE_WORLD_EVENT_POINTER(EntityDestroyedEvent);

/**
 * @warning only entities with non-null parant cound send this event.
 */
struct EntityParentChangedEvent : public EntityEvent {
	virtual WorldEventType GetEventType() const { return WorldEventTypeEntityParentChanged; }
};

DEFINE_WORLD_EVENT_POINTER(EntityParentChangedEvent);

/**
 * @warning only entities with non-null parant cound send this event.
 */
struct EntityActiveChangedEvent : public EntityEvent {
	virtual WorldEventType GetEventType() const { return WorldEventTypeEntityActiveChanged; }
};

DEFINE_WORLD_EVENT_POINTER(EntityActiveChangedEvent);

/**
 * @warning only entities with non-null parant cound send this event.
 */
struct EntityTagChangedEvent : public EntityEvent {
	virtual WorldEventType GetEventType() const { return WorldEventTypeEntityTagChanged; }
};

DEFINE_WORLD_EVENT_POINTER(EntityTagChangedEvent);

/**
 * @warning only entities with non-null parant cound send this event.
 */
struct EntityNameChangedEvent : public EntityEvent {
	virtual WorldEventType GetEventType() const { return WorldEventTypeEntityNameChanged; }
};

DEFINE_WORLD_EVENT_POINTER(EntityNameChangedEvent);

struct CameraDepthChangedEvent : public EntityEvent {
	virtual WorldEventType GetEventType() const { return WorldEventTypeCameraDepthChanged; }
};

DEFINE_WORLD_EVENT_POINTER(CameraDepthChangedEvent);

DEFINE_WORLD_EVENT_POINTER(EntityTransformChangedEvent);
struct EntityTransformChangedEvent : public EntityEvent {
	virtual WorldEventType GetEventType() const { return WorldEventTypeEntityTransformChanged; }

	virtual bool Compare(WorldEventBasePointer other) const {
		return prs < suede_static_cast<EntityTransformChangedEventPointer>(other)->prs;
	}

	// Hw: local or world.
	// Lw: position rotation or scale.
	uint prs;
};

#undef DEFINE_WORLD_EVENT_POINTER

class WorldEventListener {
public:
	virtual void OnWorldEvent(WorldEventBasePointer e) = 0;
};

class WorldEntityWalker {
public:
	enum WalkCommand {
		WalkCommandNext,
		WalkCommandBreak,
		WalkCommandContinue,
	};

	virtual WalkCommand OnWalkEntity(Entity entity) = 0;
};

struct Decal;
class GizmosPainter;

class SUEDE_API IWorld : virtual public IObject {
public:
	virtual void Update() = 0;
	virtual Object Create(ObjectType type) = 0;

	virtual Entity Import(const std::string& path, EntityLoadedListener* listener) = 0;

	virtual Transform GetRootTransform() = 0;

	virtual Camera GetMainCamera() = 0;
	virtual void SetMainCamera(Camera value) = 0;

	virtual RenderTexture GetScreenRenderTarget() = 0;

	virtual Entity GetEntity(uint id) = 0;
	virtual bool GetEntities(ObjectType type, std::vector<Entity>& entities) = 0;
	virtual void WalkEntityHierarchy(WorldEntityWalker* walker) = 0;

	virtual bool FireEvent(WorldEventBasePointer e) = 0;
	virtual void FireEventImmediate(WorldEventBasePointer e) = 0;
	virtual void AddEventListener(WorldEventListener* listener) = 0;
	virtual void RemoveEventListener(WorldEventListener* listener) = 0;

	virtual void GetDecals(std::vector<Decal*>& container) = 0;

	virtual Environment GetEnvironment() = 0;
};

SUEDE_DEFINE_OBJECT_POINTER(World);

SUEDE_API World& WorldInstance();
