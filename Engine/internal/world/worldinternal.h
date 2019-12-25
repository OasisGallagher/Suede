#pragma once
#include <set>
#include <map>
#include <mutex>
#include <thread>

#include "world.h"
#include "light.h"
#include "camera.h"
#include "screen.h"
#include "projector.h"
#include "gameobject.h"
#include "gizmospainter.h"
#include "containers/sortedvector.h"

#include "tools/event.h"

class SceneInternal;
class Sample;
class RenderingContext;
class DecalCreater;
class GameObjectLoaderThreadPool;

class WorldInternal {
public:
	WorldInternal();
	~WorldInternal();

public:
	void Initialize();

	void Update();
	void CullingUpdate();

	void Finalize();

	Environment* GetEnvironment() { return environment_; }
	const FrameStatistics* GetFrameStatistics();
	Transform* GetRootTransform();

	void DestroyGameObject(uint id);
	void DestroyGameObject(GameObject* go);

	GameObject* Import(const std::string& path);
	GameObject* Import(const std::string& path, Lua::Func<void, GameObject*, const std::string&> callback);

	void ImportTo(GameObject* go, const std::string& path);

	GameObject* GetGameObject(uint id);
	std::vector<GameObject*> GetGameObjectsOfComponent(suede_guid guid);

	void WalkGameObjectHierarchy(GameObjectWalker* walker);
	void GetDecals(std::vector<Decal>& container);

private:
	void UpdateTimeUniformBuffer();

private:
	std::thread::id threadId_;
	RenderingContext* context_;

	SceneInternal* scene_;
	Environment* environment_;

	GameObjectLoaderThreadPool* importer_;

	std::mutex hierarchyMutex_;
};
