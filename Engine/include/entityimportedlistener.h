#pragma once
#include "entity.h"

class EntityImportedListener {
public:
	virtual void OnEntityImported(Entity root) = 0;
};
