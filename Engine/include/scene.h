#pragma once
#include "subsystem.h"
#include "material.h"
#include "gameobject.h"

struct Environment {
	ref_ptr<Material> skybox;
	float fogDensity = 0.0001f;

	Color fogColor = Color::white;
	Color ambientColor = Color::white * 0.02;
};

enum class WalkCommand {
	Next,
	Break,
	Continue,
};

struct Decal;
class SUEDE_API Scene : public Subsystem {
	SUEDE_DECLARE_IMPLEMENTATION(Scene)
public:
	enum {
		SystemType = SubsystemType::Scene,
	};

public:
	Scene();

public:
	GameObject* GetGameObject(uint id);
	void DestroyGameObject(uint id);
	void DestroyGameObject(GameObject* go);
	std::vector<GameObject*> GetGameObjectsOfComponent(suede_guid guid);
	void WalkGameObjectHierarchy(std::function<WalkCommand(GameObject*)> walker);

	Environment* GetEnvironment();
	void GetDecals(std::vector<Decal>& container);

	Transform* GetRootTransform();
	template <class T> std::vector<T*> GetComponents();

	void Import(const std::string& path, std::function<void(GameObject*, const std::string&)> callback);

protected:
	virtual void Awake();
	virtual void Update(float deltaTime);
	virtual void CullingUpdate(float deltaTime);
};

template <class T> std::vector<T*> Scene::GetComponents() {
	std::vector<T*> components;
	for (GameObject* go : GetGameObjectsOfComponent(T::GetComponentGUID())) {
		components.push_back(go->GetComponent<T>());
	}

	return components;
}
