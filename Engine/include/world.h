#pragma once
#include <vector>

#include "object.h"
#include "camera.h"
#include "material.h"
#include "subsystem.h"
#include "transform.h"
#include "gameobject.h"

#include "tools/event.h"
#include "tools/singleton.h"

enum class WalkCommand {
	Next,
	Break,
	Continue,
};

struct Environment {
	ref_ptr<Material> skybox;
	float fogDensity = 0.0001f;

	Color fogColor = Color::white;
	Color ambientColor = Color::white * 0.02;
};

struct FrameStatistics {
	uint ndrawcalls;
	uint nvertices;
	uint ntriangles;

	float frameRate;

	double scriptElapsed;
	double cullingElapsed;
	double renderingElapsed;
};

struct Decal;
class SUEDE_API World : private Singleton2<World> {
	friend class Singleton<World>;
	SUEDE_DECLARE_IMPLEMENTATION(World)

public:
	static void Initialize();
	static void Finalize();

	static void Update();
	static void CullingUpdate();

	static Environment* GetEnvironment();
	static const FrameStatistics* GetFrameStatistics();

	static GameObject* GetGameObject(uint id);
	static GameObject* Import(const std::string& path, std::function<void(GameObject*, const std::string&)> callback);

	static void DestroyGameObject(uint id);
	static void DestroyGameObject(GameObject* go);

	static Transform* GetRootTransform();
	static void GetDecals(std::vector<Decal>& container);

	static void WalkGameObjectHierarchy(std::function<WalkCommand(GameObject*)> walker);

	static Subsystem* GetSubsystem(SubsystemType type);
	template <class T> static T* GetSubsystem();

public:
	template <class T>
	static std::vector<T*> GetComponents();
	static std::vector<GameObject*> GetGameObjectsOfComponent(suede_guid guid);

public:
	static sorted_event<>& frameEnter();
	static sorted_event<>& frameLeave();

private:
	World();
};

template <class T> std::vector<T*> World::GetComponents() {
	std::vector<T*> components;
	for (GameObject* go : GetGameObjectsOfComponent(T::GetComponentGUID())) {
		components.push_back(go->GetComponent<T>());
	}

	return components;
}

template <class T> T* World::GetSubsystem() {
	return dynamic_cast<T*>(GetSubsystem(T::SystemType));
}
