#pragma once
#include "entity.h"

class EntityLoadedListener {
public:
	virtual void OnEntityImported(Entity root, const std::string& path) = 0;
};

