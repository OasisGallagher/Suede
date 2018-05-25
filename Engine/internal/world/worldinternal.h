#pragma once
#include <set>
#include <ZThread/Mutex.h>

#include "plane.h"
#include "world.h"
#include "light.h"
#include "camera.h"
#include "screen.h"
#include "entity.h"
#include "projector.h"
#include "environment.h"
#include "containers/freelist.h"
#include "containers/sortedvector.h"
#include "internal/base/objectinternal.h"

class Sample;
class AsyncEntityImporter;

class WorldInternal : public ObjectInternal, public IWorld, public ScreenSizeChangedListener, public WorldEventListener {
	DEFINE_FACTORY_METHOD(World)

public:
	WorldInternal();
	~WorldInternal();

public:
	virtual void Update();

	virtual Transform GetRootTransform() { return root_->GetTransform(); }
	virtual Object Create(ObjectType type);

	virtual Entity Import(const std::string& path, EntityImportedListener* listener);

	virtual Camera GetMainCamera() { return mainCamera_; }
	virtual void SetMainCamera(Camera value) { mainCamera_ = value; }

	// TODO: how to get screen framebuffer?
	virtual RenderTexture GetScreenRenderTarget() { return screenRenderTarget_; }

	virtual Entity GetEntity(uint id);
	virtual bool GetEntities(ObjectType type, std::vector<Entity>& entities, EntitySelector* selector);
	virtual bool GetVisibleEntities(std::vector<Entity>& entities, const glm::mat4& worldToClipMatrix);

	virtual bool FireEvent(WorldEventBasePointer e);
	virtual void FireEventImmediate(WorldEventBasePointer e);
	virtual void AddEventListener(WorldEventListener* listener);
	virtual void RemoveEventListener(WorldEventListener* listener);

	virtual void GetDecals(std::vector<Decal*>& container);

	virtual Environment GetEnvironment() { return environment_; }

public:
	virtual void OnScreenSizeChanged(uint width, uint height);

public:
	virtual void OnWorldEvent(WorldEventBasePointer e);

private:
	friend void InitWorld(WorldInternal* world);

private:
	void FireEvents();
	void RenderUpdate();
	void UpdateDecals();
	void UpdateEntities();

	void CreateDecals(Camera camera);
	bool CreateProjectorDecal(Camera camera, Projector p, Plane planes[6]);
	bool CreateEntityDecal(Camera camera, Decal& decal, Entity entity, Plane planes[6]);
	bool ClampMesh(Camera camera, std::vector<glm::vec3>& triangles, Entity entity, Plane planes[6]);

	void UpdateTimeUniformBuffer();

	bool IsVisible(Entity entity, const glm::mat4& worldToClipMatrix);
	bool FrustumCulling(const Bounds& bounds, const glm::mat4& worldToClipMatrix);
	void GetRenderableEntitiesInHierarchy(std::vector<Entity>& entities, Transform root, const glm::mat4& worldToClipMatrix);

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
	typedef std::vector<WorldEventListener*> EventListenerContainer;
	typedef std::set<Projector, ProjectorComparer> ProjectorContainer;
	typedef std::set<WorldEventBasePointer, WorldEventComparer> WorldEventCollection;
	typedef WorldEventCollection WorldEventContainer[WorldEventTypeCount];

private:
	Entity root_;
	Camera mainCamera_;
	RenderTexture screenRenderTarget_;

	LightContainer lights_;
	CameraContainer cameras_;

	AsyncEntityImporter* importer_;

	Plane planes_[6];
	DecalContainer decals_;
	ProjectorContainer projectors_;

	EntityDictionary entities_;
	EventListenerContainer listeners_;

	WorldEventContainer events_;
	ZThread::Mutex eventContainerMutex_;

	Sample *update_entities, *update_decals, *update_rendering;

	Environment environment_;
};
