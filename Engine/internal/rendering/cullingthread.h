#pragma once
#include "scene.h"
#include "bounds.h"
#include "gameobject.h"

#include "tools/event.h"
#include "internal/async/threadpool.h"

class Time;
class Scene;
class Plane;
class Profiler;
class Renderer;
class RenderingContext;

class CullingTask : public Task {
public:
	CullingTask(RenderingContext* context);
	~CullingTask();

public:
	void SetCullingMask(int value) { cullingMask_ = value; }
	void SetWorldToClipMatrix(const Matrix4& value);
	std::vector<GameObject*>& GetGameObjects() { return visibleGameObjects_; }

public:
	event<> finished;

private:
	virtual void Run() override;

private:
	bool IsVisible(Renderer* renderer);

private:
	int cullingMask_ = -1;
	uint64 lastTimeStamp_ = 0;

	RenderingContext* context_;

	Time* time_;
	Scene* scene_;
	Profiler* profiler_;

	Plane* frustumPlanes_;
	std::vector<GameObject*> visibleGameObjects_;
};

class CullingThread : public ThreadPool {
public:
	CullingThread(RenderingContext* context) : ThreadPool(4) {}
};
