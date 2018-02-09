#pragma once
#include <vector>
#include "entity.h"
#include "object.h"
#include "transform.h"
#include "environment.h"

enum WorldEventType {
	WorldEventTypeEntityCreated,
	WorldEventTypeEntityDestroyed,
	WorldEventTypeEntityTagChanged,
	WorldEventTypeEntityNameChanged,
	WorldEventTypeEntityParentChanged,
	WorldEventTypeEntityTransformChanged,
	WorldEventTypeEntityActive,

	WorldEventTypeCustom = 128,
};

#define DEFINE_WORLD_EVENT_POINTER(type)	typedef std::shared_ptr<struct type> type ## Pointer

template <class Ptr>
Ptr NewWorldEvent() { return std::make_shared<Ptr::element_type>(); }

DEFINE_WORLD_EVENT_POINTER(WorldEventBase);
struct WorldEventBase {
	virtual WorldEventType GetEventType() const = 0;

	virtual bool Equals(WorldEventBasePointer other) const {
		return GetEventType() == other->GetEventType();
	}
};

DEFINE_WORLD_EVENT_POINTER(EntityEvent);
struct EntityEvent : public WorldEventBase {
	Entity entity;
	virtual bool Equals(WorldEventBasePointer other) const {
		return WorldEventBase::Equals(other) && entity == ssp_cast<EntityEventPointer>(other)->entity;
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

struct EntityParentChangedEvent : public EntityEvent {
	virtual WorldEventType GetEventType() const { return WorldEventTypeEntityParentChanged; }
};

DEFINE_WORLD_EVENT_POINTER(EntityParentChangedEvent);

struct EntityActiveEvent : public EntityEvent {
	virtual WorldEventType GetEventType() const { return WorldEventTypeEntityActive; }
};

DEFINE_WORLD_EVENT_POINTER(EntityActiveEvent);

struct EntityTagChangedEvent : public EntityEvent {
	virtual WorldEventType GetEventType() const { return WorldEventTypeEntityTagChanged; }
};

DEFINE_WORLD_EVENT_POINTER(EntityTagChangedEvent);

struct EntityNameChangedEvent : public EntityEvent {
	virtual WorldEventType GetEventType() const { return WorldEventTypeEntityNameChanged; }
};

DEFINE_WORLD_EVENT_POINTER(EntityNameChangedEvent);

DEFINE_WORLD_EVENT_POINTER(EntityTransformChangedEvent);
struct EntityTransformChangedEvent : public EntityEvent {
	virtual WorldEventType GetEventType() const { return WorldEventTypeEntityTransformChanged; }

	virtual bool Equals(WorldEventBasePointer other) const {
		return EntityEvent::Equals(other) && prs == ssp_cast<EntityTransformChangedEventPointer>(other)->prs;
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

class SUEDE_API IWorld : virtual public IObject {
public:
	virtual void Update() = 0;
	virtual Object Create(ObjectType type) = 0;

	virtual Entity Import(const std::string& path) = 0;

	virtual Transform GetRootTransform() = 0;
	virtual Entity GetEntity(uint id) = 0;
	virtual bool GetEntities(ObjectType type, std::vector<Entity>& entities) = 0;

	virtual bool FireEvent(WorldEventBasePointer e) = 0;
	virtual void FireEventImmediate(WorldEventBasePointer e) = 0;
	virtual void AddEventListener(WorldEventListener* listener) = 0;
	virtual void RemoveEventListener(WorldEventListener* listener) = 0;

	virtual Environment GetEnvironment() = 0;
};

SUEDE_DEFINE_OBJECT_POINTER(World);

SUEDE_API World& WorldInstance();
