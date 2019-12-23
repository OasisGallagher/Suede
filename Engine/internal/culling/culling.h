#pragma once
#include <mutex>

#include "world.h"
#include "gameobject.h"

#include "tools/event.h"

class Culling;

class Culling : public WorldGameObjectWalker {
public:
	Culling();
	~Culling();

public:
	std::vector<GameObject*>& GetGameObjects() { return gameObjects_; }

	void Stop();
	bool IsWorking() { return !stopped_ && working_; }

	void Cull(const Matrix4& worldToClipMatrix);

public:
	event<> cullingFinished;

public:
	virtual void Run();
	virtual WalkCommand OnWalkGameObject(GameObject* go);

private:
	bool IsVisible(GameObject* go, const Matrix4& worldToClipMatrix);
	bool FrustumCulling(const Bounds & bounds, const Matrix4& worldToClipMatrix);

private:
	std::mutex mutex_;
	std::thread thread_;
	std::condition_variable cond_;

	bool working_, stopped_;
	Matrix4 worldToClipMatrix_;
	std::vector<GameObject*> gameObjects_;
};
