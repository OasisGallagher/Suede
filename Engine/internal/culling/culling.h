#pragma once
#include "entity.h"
#include "internal/async/async.h"
#include "internal/async/threadpool.h"

class CullingListener {
public:
	virtual void OnCullingFinished() = 0;
};

class Culling : public AsyncWorker {
public:
	Culling(const glm::mat4& worldToClipMatrix, AsyncEventReceiver* receiver);
	~Culling() {}

public:
	std::vector<Entity>& GetEntities() { return entities_; }

protected:
	virtual void OnRun();

private:
	bool IsVisible(Entity entity, const glm::mat4& worldToClipMatrix);
	bool FrustumCulling(const Bounds & bounds, const glm::mat4& worldToClipMatrix);
	void GetRenderableEntitiesInHierarchy(std::vector<Entity>& entities, Transform root, const glm::mat4& worldToClipMatrix);

private:
	glm::mat4 worldToClipMatrix_;
	std::vector<Entity> entities_;
};

class AsyncCulling : public ThreadPool {
public:
	void SetCullingListener(CullingListener* listener);

protected:
	virtual void OnSchedule(ZThread::Task& schedule);

private:
	CullingListener* listener_;
};
