#pragma once
#include <vector>
#include "sprite.h"
#include "object.h"
#include "environment.h"

enum WorldEventType {
	WorldEventTypeSpriteCreated,
	WorldEventTypeSpriteDestroyed,
	WorldEventTypeSpriteTagChanged,
	WorldEventTypeSpriteNameChanged,
	WorldEventTypeSpriteParentChanged,
	WorldEventTypeSpriteTransformChanged,
	WorldEventTypeSpriteActive,

	WorldEventTypeCustom = 128,
};

struct WorldEventBase {
	virtual WorldEventType GetEventType() const = 0;
};

struct SpriteEvent : public WorldEventBase {
	Sprite sprite;
};

struct SpriteCreatedEvent : public SpriteEvent {
	virtual WorldEventType GetEventType() const { return WorldEventTypeSpriteCreated; }
};

struct SpriteDestroyedEvent : public SpriteEvent {
	virtual WorldEventType GetEventType() const { return WorldEventTypeSpriteDestroyed; }
};

struct SpriteParentChangedEvent : public SpriteEvent {
	virtual WorldEventType GetEventType() const { return WorldEventTypeSpriteParentChanged; }
};

struct SpriteActiveEvent : public SpriteEvent {
	virtual WorldEventType GetEventType() const { return WorldEventTypeSpriteActive; }
};

struct SpriteTagChangedEvent : public SpriteEvent {
	virtual WorldEventType GetEventType() const { return WorldEventTypeSpriteTagChanged; }
};

struct SpriteNameChangedEvent : public SpriteEvent {
	virtual WorldEventType GetEventType() const { return WorldEventTypeSpriteNameChanged; }
};

struct SpriteTransformChangedEvent : public SpriteEvent {
	virtual WorldEventType GetEventType() const { return WorldEventTypeSpriteTransformChanged; }
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

	virtual Sprite Import(const std::string& path) = 0;

	virtual Sprite GetRootSprite() = 0;
	virtual Sprite GetSprite(uint id) = 0;
	virtual bool GetSprites(ObjectType type, std::vector<Sprite>& sprites) = 0;

	virtual void FireEvent(const WorldEventBase* e) = 0;
	virtual void AddEventListener(WorldEventListener* listener) = 0;
	virtual void RemoveEventListener(WorldEventListener* listener) = 0;

	virtual Environment GetEnvironment() = 0;
};

SUEDE_DEFINE_OBJECT_POINTER(World);

SUEDE_API World& WorldInstance();
