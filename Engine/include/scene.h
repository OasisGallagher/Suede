#pragma once
#include "subsystem.h"
#include "gameobject.h"

enum class WalkCommand {
	Next,
	Break,
	Continue,
};

class GameObjectWalker {
public:
	virtual WalkCommand OnWalkGameObject(GameObject* go) = 0;
};

class SUEDE_API Scene : public Subsystem {
public:
	void OnWorldEvent(WorldEventBasePtr e);

	void GetDecals(std::vector<Decal>& container);
	GameObject* GetGameObject(uint id);
	void DestroyGameObject(uint id);
	void DestroyGameObject(GameObject* go);
	std::vector<GameObject*> GetGameObjectsOfComponent(suede_guid guid);
	void WalkGameObjectHierarchy(GameObjectWalker* walker);
	Transform* GetRootTransform() { return root_->GetTransform(); }

protected:
	virtual void Awake();
	virtual void Update();
	virtual void CullingUpdate();
};
