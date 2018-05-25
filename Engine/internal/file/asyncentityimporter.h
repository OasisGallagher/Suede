#pragma once
#include <ZThread/LockedQueue.h>
#include <ZThread/PoolExecutor.h>

#include "entity.h"
#include "engine.h"
#include "entityassetloader.h"
#include "entityimportedlistener.h"

class AsyncEntityImporter : public AssetLoadedListener, public FrameEventListener {
public:
	AsyncEntityImporter();
	~AsyncEntityImporter();

public:
	void SetImportedListener(EntityImportedListener* listener);

public:
	virtual void OnLoadFinished(EntityAssetLoader* loader);

public:
	virtual void OnFrameEnter();

public:
	Entity Import(const std::string& path);
	bool ImportTo(Entity entity, const std::string& path);

private:
	ZThread::PoolExecutor executor_;
	ZThread::LockedQueue<ZThread::Task, ZThread::Mutex> schedules_;

	std::vector<ZThread::Task> tasks_;
	ZThread::Mutex taskContainerMutex_;

	EntityImportedListener* listener_;
};
