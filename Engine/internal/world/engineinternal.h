#include "engine.h"
#include <vector>

class SUEDE_API EngineInternal : public Engine {
public:
	EngineInternal() {}

public:
	virtual bool Startup(uint width, uint height);
	virtual void Shutdown();

	virtual void Update();

	virtual void AddFrameEventListener(FrameEventListener* listener);
	virtual void RemoveFrameEventListener(FrameEventListener* listener);

private:
	void SortFrameEventListeners();

private:
	typedef std::vector<FrameEventListener*> FrameEventListenerContainer;
	FrameEventListenerContainer frameEventListeners_;
};
