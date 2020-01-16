#pragma once
#include "subsystem.h"
#include "tools/event.h"

enum class FrameEventQueue {
	Time = -1000,
	Profiler,
	Statistics,
	Physics,
	CullingThread,
	MainThreadEvents,

	User = 0,

	Max = 1000,
	Gizmos,
	Inputs,
};

struct SUEDE_API FrameEvents : public Subsystem {
	enum {
		SystemType = SubsystemType::FrameEvents
	};

	FrameEvents() : Subsystem(nullptr) {}

	priority_event<> frameEnter;
	priority_event<> frameLeave;
};
