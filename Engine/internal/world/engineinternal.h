#include "engine.h"
#include <vector>

struct lua_State;
class EngineInternal {
public:
	EngineInternal() : started_(false) {}

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
	bool started_;
	lua_State* L;
};
