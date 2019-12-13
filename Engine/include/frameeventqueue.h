#pragma once

enum class FrameEventQueue {
	Time = -1000,
	Profiler,
	Statistics,
	Physics,

	User = 0,

	Max = 1000,
	Gizmos,
	Inputs,
};
