#pragma once
#include "scene.h"
#include "bounds.h"
#include "gameobject.h"

#include "tools/event.h"
#include "internal/async/worker.h"

class Time;
class Scene;
class Profiler;
class RenderingContext;

class CullingThread : public intrusive_ref_counter, public Worker {
public:
	CullingThread(RenderingContext* context);

public:
	void Cull(const Matrix4& worldToClipMatrix);
	std::vector<GameObject*>& GetGameObjects() { return gameObjects_; }

public:
	event<> cullingFinished;

public:
	virtual bool OnWork();

private:
	bool IsVisible(GameObject* go, const Matrix4& worldToClipMatrix);
	bool FrustumCulling(const Bounds& bounds, const Matrix4& worldToClipMatrix);
	WalkCommand OnWalkGameObject(GameObject* go);

private:
	int cullingUpdateFrame_ = -1;
	uint64 lastTimeStamp_ = 0;

	RenderingContext* context_;

	Time* time_;
	Scene* scene_;
	Profiler* profiler_;
	
	Matrix4 worldToClipMatrix_;
	std::vector<GameObject*> gameObjects_;
};
