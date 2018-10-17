#include "engine.h"
#include <vector>

struct lua_State;
class EngineInternal : public Engine {
public:
	EngineInternal() {}

public:
	virtual bool Startup(uint width, uint height);
	virtual void Shutdown();

	virtual void Update();

	virtual void AddFrameEventListener(FrameEventListener* listener);
	virtual void RemoveFrameEventListener(FrameEventListener* listener);

private:
	void LuaUpdate();
	void SortFrameEventListeners();

private:
	typedef std::vector<FrameEventListener*> FrameEventListenerContainer;
	FrameEventListenerContainer frameEventListeners_;

private:
	lua_State* L;
};
