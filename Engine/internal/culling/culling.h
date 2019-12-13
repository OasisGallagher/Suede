#pragma once
#include <ZThread/Mutex.h>
#include <ZThread/Runnable.h>
#include <ZThread/Condition.h>

#include "world.h"
#include "gameobject.h"

#include "tools/event.h"

class Culling;

class Culling : public ZThread::Runnable, public WorldGameObjectWalker {
public:
	Culling();
	~Culling() {}

public:
	std::vector<GameObject*>& GetGameObjects() { return gameObjects_; }

	void Stop();
	bool IsWorking() { return !stopped_ && working_; }

	void Cull(const Matrix4& worldToClipMatrix);

public:
	event<> cullingFinished;

public:
	virtual WalkCommand OnWalkGameObject(GameObject* go);

protected:
	virtual void run();

private:
	bool IsVisible(GameObject* go, const Matrix4& worldToClipMatrix);
	bool FrustumCulling(const Bounds & bounds, const Matrix4& worldToClipMatrix);

private:
	ZThread::Mutex mutex_;
	ZThread::Condition cond_;

	bool working_, stopped_;
	Matrix4 worldToClipMatrix_;
	std::vector<GameObject*> gameObjects_;
};
