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
#include "containers/sortedvector.h"
#include "internal/base/objectinternal.h"

class Sample;
class DecalCreater;
class EntityLoaderThreadPool;

class WorldInternal : public World, public ScreenSizeChangedListener, public WorldEventListener {
public:
	WorldInternal();
	~WorldInternal();

public:
	virtual void Initialize();
	virtual void CullingUpdate();
	virtual void RenderingUpdate();
	virtual void Finalize();

	virtual Transform GetRootTransform() { return root_->GetTransform(); }

	virtual Object CreateObject(ObjectType type);

	virtual void DestroyEntity(uint id);
	virtual void DestroyEntity(Entity entity);

	virtual Entity Import(const std::string& path, EntityLoadedListener* listener);
	virtual bool ImportTo(Entity entity, const std::string& path, EntityLoadedListener* listener);

	virtual Entity GetEntity(uint id);
	virtual bool GetEntities(ObjectType type, std::vector<Entity>& entities);

	virtual void WalkEntityHierarchy(WorldEntityWalker* walker);

	virtual bool FireEvent(WorldEventBasePointer e);
	virtual void FireEventImmediate(WorldEventBasePointer e);
	virtual void AddEventListener(WorldEventListener* listener);
	virtual void RemoveEventListener(WorldEventListener* listener);

	virtual void GetDecals(std::vector<Decal>& container);

public:
	virtual void OnScreenSizeChanged(uint width, uint height);

public:
	virtual void OnWorldEvent(WorldEventBasePointer e);

private:
	void AddObject(Object object);
	bool CollectEntities(ObjectType type, std::vector<Entity>& entities);

	void OnEntityParentChanged(Entity entity);

	void FireEvents();
	void UpdateDecals();
	void CullingUpdateEntities();
	void RenderingUpdateEntities();

	void DestroyEntityRecursively(Transform root);
	bool WalkEntityHierarchyRecursively(Transform root, WorldEntityWalker* walker);

	void UpdateTimeUniformBuffer();

	void RemoveEntityFromSequence(Entity entity);
	void AddEntityToUpdateSequence(Entity entity);

private:
	struct LightComparer { bool operator() (const Light& lhs, const Light& rhs) const; };
	struct CameraComparer { bool operator() (const Camera& lhs, const Camera& rhs) const; };
	struct ProjectorComparer { bool operator() (const Projector& lhs, const Projector& rhs) const; };
	struct WorldEventComparer {
		bool operator () (const WorldEventBasePointer& lhs, const WorldEventBasePointer& rhs) const {
			return lhs->Compare(rhs);
		}
	};

	typedef sorted_vector<Entity> EntitySequence;
	typedef std::map<uint, Entity> EntityDictionary;
	typedef std::set<Light, LightComparer> LightContainer;
	typedef sorted_vector<Camera, CameraComparer> CameraContainer;
	typedef std::vector<WorldEventListener*> EventListenerContainer;
	typedef std::set<Projector, ProjectorComparer> ProjectorContainer;
	typedef std::set<WorldEventBasePointer, WorldEventComparer> WorldEventCollection;
	typedef WorldEventCollection WorldEventContainer[WorldEventTypeCount];

private:
	Entity root_;

	LightContainer lights_;
	CameraContainer cameras_;

	DecalCreater* decalCreater_;
	EntityLoaderThreadPool* importer_;

	ProjectorContainer projectors_;

	EntitySequence cullingUpdateSequence_;
	EntitySequence renderingUpdateSequence_;

	EntityDictionary entities_;
	EventListenerContainer listeners_;

	ZThread::Mutex eventsMutex_;
	WorldEventContainer events_;

	ZThread::Mutex hierarchyMutex_;
	ZThread::Mutex eventContainerMutex_;
};
