#pragma once
#include "entity.h"
#include "internal/async/async.h"
#include "internal/async/threadpool.h"

class Culling;
class CullingListener {
public:
	virtual void OnCullingFinished(Culling* worker) = 0;
};

class Culling : public AsyncWorker, public WorldEntityWalker {
public:
	Culling(const glm::mat4& worldToClipMatrix, AsyncEventListener* receiver);
	~Culling() {}

public:
	std::vector<Entity>& GetEntities() { return entities_; }

public:
	virtual WalkCommand OnWalkEntity(Entity entity);

protected:
	virtual void OnRun();

private:
	bool IsVisible(Entity entity, const glm::mat4& worldToClipMatrix);
	bool FrustumCulling(const Bounds & bounds, const glm::mat4& worldToClipMatrix);

private:
	glm::mat4 worldToClipMatrix_;
	std::vector<Entity> entities_;
};

class CullingThreadPool : public ThreadPool {
public:
	CullingThreadPool();

public:
	void GetVisibleEntities(const glm::mat4& worldToClipMatrix);

public:
	void SetCullingListener(CullingListener* listener);

protected:
	virtual void OnSchedule(ZThread::Task& schedule);

private:
	CullingListener* listener_;
};
