#pragma once
#include <set>

#include "world.h"
#include "light.h"
#include "camera.h"
#include "entity.h"
#include "environment.h"
#include "internal/base/objectinternal.h"

class WorldInternal : public ObjectInternal, public IWorld {
	DEFINE_FACTORY_METHOD(World)

public:
	WorldInternal();
	~WorldInternal() {}

public:
	virtual void Update();
	virtual Entity GetRootEntity() { return root_; }
	virtual Object Create(ObjectType type);

	virtual Entity Import(const std::string& path);

	virtual Entity GetEntity(uint id);
	virtual bool GetEntities(ObjectType type, std::vector<Entity>& entities);

	virtual void FireEvent(const WorldEventBase* e);
	virtual void AddEventListener(WorldEventListener* listener);
	virtual void RemoveEventListener(WorldEventListener* listener);

	virtual Environment GetEnvironment() { return environment_; }

private:
	struct LightComparer { bool operator() (const Light& lhs, const Light& rhs) const; };
	struct CameraComparer { bool operator() (const Camera& lhs, const Camera& rhs) const; };

	typedef std::map<uint, Entity> EntityContainer;
	typedef std::set<Light, LightComparer> LightContainer;
	typedef std::set<Camera, CameraComparer> CameraContainer;
	typedef std::vector<WorldEventListener*> EventListenerContainer;
private:
	Entity root_;
	LightContainer lights_;
	CameraContainer cameras_;
	EntityContainer entities_;
	EventListenerContainer listeners_;

	Environment environment_;
};
