#pragma once
#include <functional>

#include "gameobject.h"

class GameObjectLoadedListener {
public:
	virtual void OnGameObjectImported(GameObject root, const std::string& path) = 0;
};

typedef std::function<void(GameObject, const std::string&)> GameObjectLoadedCallback;
