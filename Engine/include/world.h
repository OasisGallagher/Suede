#pragma once
#include <vector>

#include "lua++.h"

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

class GameObjectWalker {
public:
	virtual WalkCommand OnWalkGameObject(GameObject* go) = 0;
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

struct Foo {
	class {
		int value;
	public:
		int & operator = (const int &i) { return value = i; }
		operator int() const { return value; }
	} alpha;

	class {
		float value;
	public:
		float & operator = (const float &f) { return value = f; }
		operator float() const { return value; }
	} bravo;
};

class SUEDE_API World : private Singleton2<World> {
	friend class Singleton<World>;
	SUEDE_DECLARE_IMPLEMENTATION(World)

public:
	static void Initialize();
	static void Finalize();

	static void Update();
	static void CullingUpdate();

	static void DestroyGameObject(uint id);
	static void DestroyGameObject(GameObject* go);

	static Environment* GetEnvironment();
	static const FrameStatistics* GetFrameStatistics();

	static GameObject* Import(const std::string& path);
	static GameObject* Import(const std::string& path, Lua::Func<void, GameObject*, const std::string&> callback);

	static void ImportTo(GameObject* go, const std::string& path);

	static Transform* GetRootTransform();

	static GameObject* GetGameObject(uint id);
	static void WalkGameObjectHierarchy(GameObjectWalker* walker);

	static void GetDecals(std::vector<Decal>& container);

	static Subsystem* GetSubsystem(SubsystemType type);
	template <class T> static T* GetSubsystem();

public:
	template <class T>
	static std::vector<T*> GetComponents();
	static std::vector<GameObject*> GetGameObjectsOfComponent(suede_guid guid);

public:
	static sorted_event<>& frameEnter();
	static sorted_event<>& frameLeave();
	// TODO:
	static event<GameObject*, const std::string&> gameObjectImported;

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
