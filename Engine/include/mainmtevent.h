#pragma once
#include "engine.h"
#include "tools/event.h"
#include "frameevents.h"

/**
 * An event system constructed and updated in main thread.
 */
template <class... Args>
class main_mt_event : public mt_event<Args...> {
	typedef mt_event<Args...> super;

public:
	main_mt_event() {
		Engine::GetSubsystem<FrameEvents>()->frameEnter.subscribe<super>(this, &super::update, (int)FrameEventQueue::MainThreadEvents);
	}

	~main_mt_event() {
		Engine::GetSubsystem<FrameEvents>()->frameEnter.unsubscribe(this);
	}
};
