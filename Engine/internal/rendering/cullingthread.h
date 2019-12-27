#pragma once
#include <mutex>

#include "scene.h"
#include "gameobject.h"

#include "tools/event.h"

class RenderingContext;

class CullingThread {
public:
	CullingThread(RenderingContext* context);
	~CullingThread();

public:
	std::vector<GameObject*>& GetGameObjects() { return gameObjects_; }

	void Stop();
	bool IsWorking() { return !stopped_ && working_; }

	void Cull(const Matrix4& worldToClipMatrix);

public:
	event<> cullingFinished;

public:
	void ThreadProc();

private:
	bool IsVisible(GameObject* go, const Matrix4& worldToClipMatrix);
	bool FrustumCulling(const Bounds & bounds, const Matrix4& worldToClipMatrix);
	WalkCommand OnWalkGameObject(GameObject* go);

private:
	std::mutex mutex_;
	std::thread thread_;
	std::condition_variable cond_;

	RenderingContext* context_;

	bool working_, stopped_;
	Matrix4 worldToClipMatrix_;
	std::vector<GameObject*> gameObjects_;
};
