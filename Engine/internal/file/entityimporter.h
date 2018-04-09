#pragma once

#include "entity.h"
#include "entityassetloader.h"

class EntityImporter : public LoaderCallback {
public:
	EntityImporter();
	~EntityImporter();

public:
	virtual void operator()();

public:
	Entity Import(const std::string& path);
	bool ImportTo(Entity entity, const std::string& path);

	void Update();

private:
	std::string StrError(int err);

private:
	int status_;
	EntityAssetLoader* loader_;
};
