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
	void SetWorldToClipMatrix(const Matrix4& value);
	std::vector<GameObject*>& GetGameObjects() { return gameObjects_; }

public:
	event<> finished;

private:
	virtual void Run() override;

private:
	bool IsVisible(Renderer* renderer);

private:
	uint64 lastTimeStamp_ = 0;

	RenderingContext* context_;

	Time* time_;
	Scene* scene_;
	Profiler* profiler_;

	Plane* frustumPlanes_;
	std::vector<GameObject*> gameObjects_;
};

class CullingThread : public ThreadPool {
public:
	CullingThread(RenderingContext* context);
	~CullingThread();

private:
	void OnFrameEnter();
	void OnFrameLeave();

private:
	class UpdateTask : public Task {
	public:
		UpdateTask();

	private:
		virtual void Run() override;

		Time* time_;
		Scene* scene_;
		Profiler* profiler_;
	};

	ref_ptr<UpdateTask> cullingUpdateTask_;

	RenderingContext* context_;
};
