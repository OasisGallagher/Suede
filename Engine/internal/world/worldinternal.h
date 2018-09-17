#pragma once
#include <set>
#include <ZThread/Mutex.h>

#include "plane.h"
#include "world.h"
#include "light.h"
#include "camera.h"
#include "screen.h"
#include "projector.h"
#include "gameobject.h"
#include "environment.h"
#include "containers/sortedvector.h"
#include "internal/base/objectinternal.h"

class Sample;
class DecalCreater;
class GameObjectLoaderThreadPool;

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

	virtual void DestroyGameObject(uint id);
	virtual void DestroyGameObject(GameObject go);

	virtual GameObject Import(const std::string& path, GameObjectLoadedListener* listener);
	virtual bool ImportTo(GameObject go, const std::string& path, GameObjectLoadedListener* listener);

	virtual GameObject GetGameObject(uint id);
	virtual std::vector<GameObject> GetGameObjectsOfComponent(suede_guid guid);

	virtual void WalkGameObjectHierarchy(WorldGameObjectWalker* walker);

	virtual void FireEvent(WorldEventBasePointer e);
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

	void OnGameObjectParentChanged(GameObject go);
	void OnGameObjectComponentChanged(GameObjectComponentChangedEventPointer e);

	template <class Container>
	void ManageGameObjectComponents(Container& container, Component component, bool added);

	bool IsLightComponentGUID(suede_guid guid);

	void FireEvents();
	void UpdateDecals();
	void CullingUpdateGameObjects();
	void RenderingUpdateGameObjects();

	void DestroyGameObjectRecursively(Transform root);
	bool WalkGameObjectHierarchyRecursively(Transform root, WorldGameObjectWalker* walker);

	void UpdateTimeUniformBuffer();

	void RemoveGameObjectFromSequence(GameObject go);
	void ManageGameObjectUpdateSequence(GameObject go);

private:
	struct LightComparer { bool operator() (const Light& lhs, const Light& rhs) const; };
	struct CameraComparer { bool operator() (const Camera& lhs, const Camera& rhs) const; };
	struct ProjectorComparer { bool operator() (const Projector& lhs, const Projector& rhs) const; };

	typedef sorted_vector<GameObject> GameObjectSequence;
	typedef std::map<uint, GameObject> GameObjectDictionary;
	typedef std::set<Light, LightComparer> LightContainer;
	typedef sorted_vector<Camera, CameraComparer> CameraContainer;
	typedef std::vector<WorldEventListener*> EventListenerContainer;
	typedef std::set<Projector, ProjectorComparer> ProjectorContainer;
	typedef std::vector<WorldEventBasePointer> WorldEventCollection;
	typedef WorldEventCollection WorldEventContainer[(int)WorldEventType::_Count];

private:
	GameObject root_;

	LightContainer lights_;
	CameraContainer cameras_;

	DecalCreater* decalCreater_;
	GameObjectLoaderThreadPool* importer_;

	ProjectorContainer projectors_;

	GameObjectSequence cullingUpdateSequence_;
	GameObjectSequence renderingUpdateSequence_;

	GameObjectDictionary gameObjects_;
	EventListenerContainer listeners_;

	ZThread::Mutex eventsMutex_;
	WorldEventContainer events_;

	ZThread::Mutex hierarchyMutex_;
	ZThread::Mutex eventContainerMutex_;
};

template <class Container>
void WorldInternal::ManageGameObjectComponents(Container& container, Component component, bool added) {
	typedef Container::value_type T;
	typedef typename T::element_type U;
	if (component->IsComponentType(U::GetComponentGUID())) {
		T target = suede_dynamic_cast<T>(component);
		if (added) {
			container.insert(target);
		}
		else {
			container.erase(target);
		}
	}
}
