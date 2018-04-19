#pragma once
#include <set>
#include <OpenThreads/Mutex>

#include "plane.h"
#include "world.h"
#include "light.h"
#include "camera.h"
#include "entity.h"
#include "projector.h"
#include "environment.h"
#include "containers/freelist.h"
#include "containers/sortedvector.h"
#include "internal/base/objectinternal.h"

class Sample;
class EntityImporter;

class WorldInternal : public ObjectInternal, public IWorld {
	DEFINE_FACTORY_METHOD(World)

public:
	WorldInternal();
	~WorldInternal();

public:
	virtual void Update();

	virtual Transform GetRootTransform() { return root_->GetTransform(); }
	virtual Object Create(ObjectType type);

	virtual Entity Import(const std::string& path);

	virtual Camera GetMainCamera() { return mainCamera_; }
	virtual void SetMainCamera(Camera value) { mainCamera_ = value; }

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

	void CreateDecals(Camera camera);
	bool CreateProjectorDecal(Camera camera, Projector p, Plane planes[6]);
	bool CreateEntityDecal(Camera camera, Decal& decal, Entity entity, Plane planes[6]);
	bool ClampMesh(Camera camera, std::vector<glm::vec3>& triangles, Entity entity, Plane planes[6]);

private:
	struct LightComparer { bool operator() (const Light& lhs, const Light& rhs) const; };
	struct CameraComparer { bool operator() (const Camera& lhs, const Camera& rhs) const; };
	struct ProjectorComparer { bool operator() (const Projector& lhs, const Projector& rhs) const; };
	struct WorldEventComparer {
		bool operator () (const WorldEventBasePointer& lhs, const WorldEventBasePointer& rhs) const {
			return lhs->Compare(rhs);
		}
	};

	typedef free_list<Decal> DecalContainer;
	typedef std::vector<Entity> EntityContainer;
	typedef std::map<uint, Entity> EntityDictionary;
	typedef std::set<Light, LightComparer> LightContainer;
	typedef sorted_vector<Camera, CameraComparer> CameraContainer;
	typedef std::set<Projector, ProjectorComparer> ProjectorContainer;
	typedef std::vector<WorldEventListener*> EventListenerContainer;
	typedef std::set<WorldEventBasePointer, WorldEventComparer> WorldEventCollection;
	typedef WorldEventCollection WorldEventContainer[WorldEventTypeCount];

private:
	Entity root_;
	Camera mainCamera_;

	LightContainer lights_;
	CameraContainer cameras_;

	EntityImporter* importer_;

	Plane planes_[6];
	DecalContainer decals_;
	ProjectorContainer projectors_;

	EntityDictionary entities_;
	EventListenerContainer listeners_;

	WorldEventContainer events_;
	OpenThreads::Mutex eventContainerMutex_;

	Sample *update_entities, *update_decals, *update_rendering;

	Environment environment_;
};
