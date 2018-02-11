#pragma once
#include <set>

#include "world.h"
#include "light.h"
#include "camera.h"
#include "entity.h"
#include "projector.h"
#include "environment.h"
#include "containers/freelist.h"
#include "internal/geometry/plane.h"
#include "internal/base/objectinternal.h"

class WorldInternal : public ObjectInternal, public IWorld {
	DEFINE_FACTORY_METHOD(World)

public:
	WorldInternal();
	~WorldInternal() {}

public:
	virtual void Update();

	virtual Transform GetRootTransform() { return root_->GetTransform(); }
	virtual Object Create(ObjectType type);

	virtual Entity Import(const std::string& path);

	virtual Entity GetEntity(uint id);
	virtual bool GetEntities(ObjectType type, std::vector<Entity>& entities);

	virtual bool FireEvent(WorldEventBasePointer e);
	virtual void FireEventImmediate(WorldEventBasePointer e);
	virtual void AddEventListener(WorldEventListener* listener);
	virtual void RemoveEventListener(WorldEventListener* listener);

	virtual void GetDecals(std::vector<Decal*>& container);

	virtual Environment GetEnvironment() { return environment_; }

private:
	void FireEvents();
	void RenderUpdate();
	void UpdateDecals();
	void UpdateEntities();

	void CreateDecals();
	bool CreateProjectorDecal(Projector p, Plane planes[6]);
	bool CreateEntityDecal(Decal& decal, Entity entity, Plane planes[6]);

	bool ClampMesh(std::vector<glm::vec3>& triangles, Entity entity, Plane planes[6]);

private:
	struct LightComparer { bool operator() (const Light& lhs, const Light& rhs) const; };
	struct CameraComparer { bool operator() (const Camera& lhs, const Camera& rhs) const; };
	struct ProjectorComparer { bool operator() (const Projector& lhs, const Projector& rhs) const; };

	typedef free_list<Decal> DecalContainer;
	typedef std::map<uint, Entity> EntityContainer;
	typedef std::set<Light, LightComparer> LightContainer;
	typedef std::set<Camera, CameraComparer> CameraContainer;
	typedef std::set<Projector, ProjectorComparer> ProjectorContainer;
	typedef std::vector<WorldEventBasePointer> WorldEventContainer;
	typedef std::vector<WorldEventListener*> EventListenerContainer;

private:
	Entity root_;
	LightContainer lights_;
	CameraContainer cameras_;

	Plane planes_[6];
	DecalContainer decals_;
	ProjectorContainer projectors_;

	EntityContainer entities_;
	EventListenerContainer listeners_;
	WorldEventContainer events_;
	Environment environment_;
};
