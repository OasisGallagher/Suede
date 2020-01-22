#pragma once
#include "tools/pimplidiom.h"

enum class SubsystemType {
	Time,
	Profiler,
	FrameEvents,

	Physics,
	Scene,

	Gizmos,
	Input,

	Tags,
	LayerMask,
	Graphics,

	_Count,
};

class SUEDE_API Subsystem : public PimplIdiom {
public:
	~Subsystem() {}

	virtual void Awake() {}
	virtual void Update(float deltaTime) {}
	virtual void OnDestroy() {}

protected:
	Subsystem(void* d);
};
