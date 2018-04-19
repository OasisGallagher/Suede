#pragma once

#include "entity.h"
#include "engine.h"
#include "entityassetloader.h"

class EntityImporter : public LoaderCallback, public FrameEventListener {
public:
	EntityImporter();
	~EntityImporter();

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
};
