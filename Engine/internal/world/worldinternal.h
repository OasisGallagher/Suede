#pragma once
#include <set>
#include <map>
#include <thread>
#include <ZThread/Mutex.h>

#include "world.h"
#include "light.h"
#include "camera.h"
#include "screen.h"
#include "projector.h"
#include "gameobject.h"
#include "gizmospainter.h"
#include "containers/sortedvector.h"

#include "tools/event.h"

class Sample;
class RenderingContext;
class DecalCreater;
class GameObjectLoaderThreadPool;

class WorldInternal : public WorldEventListener {
public:
	WorldInternal();
	~WorldInternal();

public:
	void Initialize();

	void Update();
	void CullingUpdate();

	void Finalize();

	Environment* GetEnvironment() { return environment_; }
	Transform* GetRootTransform() { return root_->GetTransform(); }

	void DestroyGameObject(uint id);
	void DestroyGameObject(GameObject* go);

	GameObject* Import(const std::string& path);
	GameObject* Import(const std::string& path, Lua::Func<void, GameObject*, const std::string&> callback);
	bool ImportTo(GameObject* go, const std::string& path);

	GameObject* GetGameObject(uint id);

	std::vector<GameObject*> GetGameObjectsOfComponent(suede_guid guid);

	void WalkGameObjectHierarchy(WorldGameObjectWalker* walker);

	void FireEvent(WorldEventBasePtr e);
	void FireEventImmediate(WorldEventBasePtr e);
	void AddEventListener(WorldEventListener* listener);
	void RemoveEventListener(WorldEventListener* listener);

	void GetDecals(std::vector<Decal>& container);

public:
	void OnWorldEvent(WorldEventBasePtr e);

private:
	void AddGameObject(GameObject* go);

	void OnGameObjectParentChanged(GameObject* go);
	void OnGameObjectComponentChanged(GameObjectComponentChangedEventPtr e);

	template <class Container>
	void ManageGameObjectComponents(Container& container, Component* component, int state);

	void FireEvents();
	void UpdateDecals();
	void CullingUpdateGameObjects();
	void RenderingUpdateGameObjects();

	void RemoveGameObject(GameObject* go);
	void DestroyGameObjectRecursively(Transform* root);

	bool WalkGameObjectHierarchyRecursively(Transform* root, WorldGameObjectWalker* walker);

	void UpdateTimeUniformBuffer();

	void RemoveGameObjectFromSequence(GameObject* go);
	void ManageGameObjectUpdateSequence(GameObject* go);

private:
	struct LightComparer { bool operator() (const ref_ptr<Light>& lhs, const ref_ptr<Light>& rhs) const; };
	struct CameraComparer { bool operator() (const ref_ptr<Camera>& lhs, const ref_ptr<Camera>& rhs) const; };
	struct ProjectorComparer { bool operator() (const ref_ptr<Projector>& lhs, const ref_ptr<Projector>& rhs) const; };

	typedef sorted_vector<ref_ptr<GizmosPainter>> GizmosPainterContainer;

	typedef sorted_vector<GameObject*> GameObjectSequence;
	typedef std::map<uint, ref_ptr<GameObject>> GameObjectDictionary;
	typedef std::set<ref_ptr<Light>, LightComparer> LightContainer;
	typedef sorted_vector<ref_ptr<Camera>, CameraComparer> CameraContainer;
	typedef std::vector<WorldEventListener*> EventListenerContainer;
	typedef std::set<ref_ptr<Projector>, ProjectorComparer> ProjectorContainer;
	typedef std::vector<WorldEventBasePtr> WorldEventCollection;
	typedef WorldEventCollection WorldEventContainer[WorldEventType::size()];

private:
	std::thread::id threadId_;
	RenderingContext* context_;

	GameObject* root_;
	Environment* environment_;

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
void WorldInternal::ManageGameObjectComponents(Container& container, Component* component, int state) {
	typedef typename Container::value_type V;
	typedef typename V::element_type T;

	if (component->IsComponentType(T::GetComponentGUID())) {
		T* target = (T*)component;
		if (state == GameObjectComponentChangedEvent::ComponentAdded) {
			container.insert(container.end(), ref_ptr<T>(target));
		}
		else if (state == GameObjectComponentChangedEvent::ComponentRemoved) {
			container.erase(ref_ptr<T>(target));
		}
	}
}
