#pragma once
#include <ZThread/Mutex.h>
#include <ZThread/Runnable.h>
#include <ZThread/Condition.h>

#include "world.h"
#include "entity.h"

class Culling;
class CullingListener {
public:
	virtual void OnCullingFinished() = 0;
};

class Culling : public ZThread::Runnable, public WorldEntityWalker {
public:
	Culling(CullingListener* listener);
	~Culling() {}

public:
	std::vector<Entity>& GetEntities() { return entities_; }

	void Stop();
	bool IsWorking() { return !stopped_ && working_; }

	void Cull(const glm::mat4& worldToClipMatrix);

public:
	virtual WalkCommand OnWalkEntity(Entity entity);

protected:
	virtual void run();

private:
	bool IsVisible(Entity entity, const glm::mat4& worldToClipMatrix);
	bool FrustumCulling(const Bounds & bounds, const glm::mat4& worldToClipMatrix);

private:
	ZThread::Mutex mutex_;
	ZThread::Condition cond_;

	bool working_, stopped_;
	CullingListener* listener_;
	glm::mat4 worldToClipMatrix_;
	std::vector<Entity> entities_;
};
