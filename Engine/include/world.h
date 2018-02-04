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

struct WorldEventBase {
	virtual WorldEventType GetEventType() const = 0;
};

struct EntityEvent : public WorldEventBase {
	Entity entity;
};

struct EntityCreatedEvent : public EntityEvent {
	virtual WorldEventType GetEventType() const { return WorldEventTypeEntityCreated; }
};

struct EntityDestroyedEvent : public EntityEvent {
	virtual WorldEventType GetEventType() const { return WorldEventTypeEntityDestroyed; }
};

struct EntityParentChangedEvent : public EntityEvent {
	virtual WorldEventType GetEventType() const { return WorldEventTypeEntityParentChanged; }
};

struct EntityActiveEvent : public EntityEvent {
	virtual WorldEventType GetEventType() const { return WorldEventTypeEntityActive; }
};

struct EntityTagChangedEvent : public EntityEvent {
	virtual WorldEventType GetEventType() const { return WorldEventTypeEntityTagChanged; }
};

struct EntityNameChangedEvent : public EntityEvent {
	virtual WorldEventType GetEventType() const { return WorldEventTypeEntityNameChanged; }
};

struct EntityTransformChangedEvent : public EntityEvent {
	virtual WorldEventType GetEventType() const { return WorldEventTypeEntityTransformChanged; }
	// Hw: local or world.
	// Lw: position rotation or scale.
	uint prs;
};

class WorldEventListener {
public:
	virtual void OnWorldEvent(const WorldEventBase* e) = 0;
};

class SUEDE_API IWorld : virtual public IObject {
public:
	virtual void Update() = 0;
	virtual Object Create(ObjectType type) = 0;

	virtual Entity Import(const std::string& path) = 0;

	virtual Transform GetRootTransform() = 0;
	virtual Entity GetEntity(uint id) = 0;
	virtual bool GetEntities(ObjectType type, std::vector<Entity>& entities) = 0;

	virtual void FireEvent(const WorldEventBase* e) = 0;
	virtual void AddEventListener(WorldEventListener* listener) = 0;
	virtual void RemoveEventListener(WorldEventListener* listener) = 0;

	virtual Environment GetEnvironment() = 0;
};

SUEDE_DEFINE_OBJECT_POINTER(World);

SUEDE_API World& WorldInstance();
