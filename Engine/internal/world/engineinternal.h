#include "engine.h"
#include <vector>

struct lua_State;
class EngineInternal {
public:	EngineInternal();

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
	int updateRef_;
	lua_State* L;

	FrameEventListenerContainer listeners_;
};
