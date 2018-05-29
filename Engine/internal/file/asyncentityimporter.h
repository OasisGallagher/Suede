#pragma once
#include <queue>

#define USE_POOL_EXECUTOR

#include <ZThread/LockedQueue.h>

#ifdef USE_POOL_EXECUTOR
#include <ZThread/PoolExecutor.h>
#else
#include <ZThread/ThreadedExecutor.h>
#endif

#include "entity.h"
#include "engine.h"
#include "entityassetloader.h"
#include "entityimportedlistener.h"

class AsyncEntityImporter : public AssetLoadedListener, public FrameEventListener {
public:
	AsyncEntityImporter();
	~AsyncEntityImporter();

public:
	virtual void OnLoadFinished(EntityAssetLoader* loader);

public:
	virtual void OnFrameEnter();

public:
	Entity Import(const std::string& path);
	bool ImportTo(Entity entity, const std::string& path);

	void SetImportedListener(EntityImportedListener* listener);

private:
	void UpdateSchedules();

private:
#ifdef USE_POOL_EXECUTOR
	ZThread::PoolExecutor executor_;
#else
	ZThread::ThreadedExecutor executor_;
#endif
	
	std::vector<ZThread::Task> tasks_;
	std::queue<ZThread::Task> schedules_;

	ZThread::Mutex scheduleContainerMutex_;

	EntityImportedListener* listener_;
};
