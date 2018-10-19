#include "engine.h"
#include <vector>

struct lua_State;
class EngineInternal {
public:
	EngineInternal() {}

public:
	bool Startup(uint width, uint height);
	void Shutdown();

	void Update();

	void AddFrameEventListener(FrameEventListener* listener);
	void RemoveFrameEventListener(FrameEventListener* listener);

private:
	void SortFrameEventListeners();
	void InvokeLuaMethod(const char* name);

private:
	typedef std::vector<FrameEventListener*> FrameEventListenerContainer;
	FrameEventListenerContainer frameEventListeners_;

private:
	lua_State* L;
};
