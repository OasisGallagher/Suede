#pragma once

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
	virtual void OnLoadFinished();

public:
	virtual void OnFrameEnter();

public:
	Entity Import(const std::string& path);
	bool ImportTo(Entity entity, const std::string& path);

private:
	std::string StrError(int err);

private:
	int status_;
	EntityAssetLoader* loader_;
	EntityImportedListener* listener_;
};
