#pragma once
#include <ZThread/Mutex.h>
#include <ZThread/Runnable.h>
#include <ZThread/Condition.h>

#include "world.h"
#include "gameobject.h"
#include "internal/codec/image.h"

class Culling;
class CullingListener {
public:
	virtual void OnCullingFinished() = 0;
};

struct DBVTCulling;
struct OcclusionBuffer;
class btCollisionObject;

class Culling : public ZThread::Runnable {
public:
	Culling(CullingListener* listener);
	~Culling();

public:
	void GetCullingBuffer(TexelMap& texels);
	const std::vector<GameObject>& GetGameObjects() { return gameObjects_; }

	void Stop();
	bool IsWorking() { return !stopped_ && working_; }

	void Cull(const glm::vec3& cameraPos, const glm::vec3& cameraForward, float farClipPlane, const glm::mat4& worldToClipMatrix);

protected:
	virtual void run();

private:
	bool IsVisible(btCollisionObject* co);

	void OcclusionCulling();
	void BulletDBVTCulling(const float* cameraPos, const float* cameraForward, float farClipPlane, const float* worldToClipMatrix);

private:
	ZThread::Mutex mutex_;
	ZThread::Condition cond_;

	bool working_, stopped_;
	CullingListener* listener_;

	glm::vec3 cameraPos_;
	glm::vec3 cameraForward_;
	float farClipPlane_;
	glm::mat4 worldToClipMatrix_;

	std::vector<GameObject> gameObjects_;

	DBVTCulling* dbvtCulling_;
	OcclusionBuffer* occlusionBuffer_;
	std::vector<btCollisionObject*> objectsInFrustum_;

	glm::ivec2 occlusionBufferSize_;
};
