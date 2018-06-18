#pragma once
#include "entity.h"
#include "internal/async/guard.h"
#include "internal/async/threadpool.h"

class CullingThread;
class CullingListener {
public:
	virtual void OnCullingFinished() = 0;
};

class CullingThread : public ZThread::Runnable, public WorldEntityWalker {
	enum {
		Waiting,
		Working,
		Finished,
	};

public:
	CullingThread(CullingListener* listener);
	~CullingThread() {}

public:
	std::vector<Entity>& GetEntities() { return entities_; }

	void Done() { status_ = Finished; }
	void Cull(const glm::mat4& worldToClipMatrix);

public:
	virtual WalkCommand OnWalkEntity(Entity entity);

protected:
	virtual void run();

private:
	bool IsVisible(Entity entity, const glm::mat4& worldToClipMatrix);
	bool FrustumCulling(const Bounds & bounds, const glm::mat4& worldToClipMatrix);

private:
	int status_;
	CullingListener* listener_;
	glm::mat4 worldToClipMatrix_;
	std::vector<Entity> entities_;
};
