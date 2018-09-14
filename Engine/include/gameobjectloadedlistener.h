#pragma once
#include "gameobject.h"

class GameObjectLoadedListener {
public:
	virtual void OnGameObjectImported(GameObject root, const std::string& path) = 0;
};

