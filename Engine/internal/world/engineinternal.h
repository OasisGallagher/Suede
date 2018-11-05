#include "engine.h"
#include <vector>

struct lua_State;
class EngineInternal {
public:
	EngineInternal();

public:
	bool Startup(uint width, uint height);
	void Shutdown();

	void Update();

	void AddFrameEventListener(FrameEventListener* listener);
	void RemoveFrameEventListener(FrameEventListener* listener);

private:
	typedef void (FrameEventListener::*Event)();
	typedef std::vector<FrameEventListener*> FrameEventListenerContainer;

private:
	template <class MemFunc>
	void WalkAllListeners(FrameEventListenerContainer& container, MemFunc f);

private:
	bool started_;
	int updateRef_;
	lua_State* L;

	FrameEventListenerContainer listeners_;
};

template <class MemFunc>
void  EngineInternal::WalkAllListeners(FrameEventListenerContainer& container, MemFunc f) {
	struct FrameEventComparer {
		bool operator()(FrameEventListener* lhs, FrameEventListener* rhs) const {
			return lhs->GetFrameEventQueue() < rhs->GetFrameEventQueue();
		}
	};

	typedef FrameEventListenerContainer::iterator Iterator;

	for (Iterator ite = container.begin(); ite != container.end(); ++ite) {
		((*ite)->*f)();

		Iterator p = std::upper_bound(container.begin(), ite, *ite, FrameEventComparer());
		std::rotate(p, ite, ite + 1);
	}
}
