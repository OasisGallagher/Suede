#pragma once

#include <map>
#include "entity.h"

class AssetDataLoader;

class AssetLoadedCallback {
	virtual void operator()() {

	}
};

class AssetImporter {
public:
	AssetImporter();
	~AssetImporter();

public:
	Entity Import(const std::string& path);
	bool ImportTo(Entity entity, const std::string& path);

private:
	AssetDataLoader* loader_;
};
