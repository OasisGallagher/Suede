#pragma once
#include "entity.h"

class EntityImportedListener {
public:
	virtual void OnEntityImported(bool status, Entity root) = 0;
};