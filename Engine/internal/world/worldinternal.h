#pragma once
#include <set>

#include "world.h"
#include "light.h"
#include "camera.h"
#include "sprite.h"
#include "environment.h"
#include "internal/base/objectinternal.h"

class WorldInternal : public ObjectInternal, public IWorld {
	DEFINE_FACTORY_METHOD(World)

public:
	WorldInternal();
	~WorldInternal() {}

public:
	virtual void Update();
	virtual Sprite GetRootSprite() { return root_; }
	virtual Object Create(ObjectType type);

	virtual Sprite Import(const std::string& path);

	virtual Sprite GetSprite(uint id);
	virtual bool GetSprites(ObjectType type, std::vector<Sprite>& sprites);

	virtual void FireEvent(const WorldEventBase* e);
	virtual void AddEventListener(WorldEventListener* listener);
	virtual void RemoveEventListener(WorldEventListener* listener);

	virtual Environment GetEnvironment() { return environment_; }

private:
	struct LightComparer { bool operator() (const Light& lhs, const Light& rhs) const; };
	struct CameraComparer { bool operator() (const Camera& lhs, const Camera& rhs) const; };

	typedef std::map<uint, Sprite> SpriteContainer;
	typedef std::set<Light, LightComparer> LightContainer;
	typedef std::set<Camera, CameraComparer> CameraContainer;
	typedef std::vector<WorldEventListener*> EventListenerContainer;
private:
	Sprite root_;
	LightContainer lights_;
	CameraContainer cameras_;
	SpriteContainer sprites_;
	EventListenerContainer listeners_;

	Environment environment_;
};
