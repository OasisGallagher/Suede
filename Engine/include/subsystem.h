#pragma once
#include "object.h"

enum class SubsystemType {
	Scene,
	Events,
};

class Subsystem : public Object {
protected:
	virtual void Awake() = 0;
	virtual void Update() = 0;
	virtual void CullingUpdate() = 0;
	virtual void OnWorldEvent(void* e) = 0;
};
