#pragma once
#include <set>
#include <map>
#include <ZThread/Mutex.h>

#include "world.h"
#include "light.h"
#include "camera.h"
#include "screen.h"
#include "projector.h"
#include "gameobject.h"
#include "gizmospainter.h"
#include "containers/sortedvector.h"

class Sample;
class DecalCreater;
class GameObjectLoaderThreadPool;

class WorldInternal : public ScreenSizeChangedListener, public WorldEventListener {
public:
	WorldInternal();
	~WorldInternal();

public:
	void Initialize();

	void Update();
	void CullingUpdate();

	void Finalize();

	Transform GetRootTransform() { return root_->GetTransform(); }

	Object CreateObject(ObjectType type);

	void DestroyGameObject(uint id);
	void DestroyGameObject(GameObject go);

	GameObject Import(const std::string& path, GameObjectImportedListener* listener);
	GameObject Import(const std::string& path, Lua::Func<void, GameObject, const std::string&> callback);
	bool ImportTo(GameObject go, const std::string& path, GameObjectImportedListener* listener);

	GameObject GetGameObject(uint id);
	std::vector<GameObject> GetGameObjectsOfComponent(suede_guid guid);

	void WalkGameObjectHierarchy(WorldGameObjectWalker* walker);

	void FireEvent(WorldEventBasePtr e);
	void FireEventImmediate(WorldEventBasePtr e);
	void AddEventListener(WorldEventListener* listener);
	void RemoveEventListener(WorldEventListener* listener);

	void GetDecals(std::vector<Decal>& container);

public:
	void OnScreenSizeChanged(uint width, uint height);

public:
	void OnWorldEvent(WorldEventBasePtr e);

private:
	void AddObject(Object object);

	void OnGameObjectParentChanged(GameObject go);
	void OnGameObjectComponentChanged(GameObjectComponentChangedEventPtr e);

	template <class Container>
	void ManageGameObjectComponents(Container& container, Component component, int state);

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

	typedef sorted_vector<GizmosPainter> GizmosPainterContainer;

	typedef sorted_vector<GameObject> GameObjectSequence;
	typedef std::map<uint, GameObject> GameObjectDictionary;
	typedef std::set<Light, LightComparer> LightContainer;
	typedef sorted_vector<Camera, CameraComparer> CameraContainer;
	typedef std::vector<WorldEventListener*> EventListenerContainer;
	typedef std::set<Projector, ProjectorComparer> ProjectorContainer;
	typedef std::vector<WorldEventBasePtr> WorldEventCollection;
	typedef WorldEventCollection WorldEventContainer[WorldEventType::size()];

private:
	GameObject root_;

	LightContainer lights_;
	CameraContainer cameras_;
	GizmosPainterContainer gizmosPainters_;

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
void WorldInternal::ManageGameObjectComponents(Container& container, Component component, int state) {
	typedef Container::value_type T;
	typedef typename T::element_type U;

	if (component->IsComponentType(U::GetComponentGUID())) {
		T target = suede_dynamic_cast<T>(component);
		if (state == GameObjectComponentChangedEvent::ComponentAdded) {
			container.insert(container.end(), target);
		}
		else if (state == GameObjectComponentChangedEvent::ComponentRemoved) {
			container.erase(target);
		}
	}
}
