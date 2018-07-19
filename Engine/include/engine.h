#pragma once
#include <vector>

#include "enginedefines.h"
#include "tools/singleton.h"
#include "frameeventlistener.h"

class GraphicsCanvas;
class Engine : public Singleton<Engine> {
	friend class Singleton<Engine>;

public:
	bool Initialize();
	void Release();

	void Update();

	void AddFrameEventListener(FrameEventListener* listener);
	void RemoveFrameEventListener(FrameEventListener* listener);

private:
	Engine() {}
	void SetDefaultGLStates();
	void SortFrameEventListeners();
	
private:
	typedef std::vector<FrameEventListener*> FrameEventListenerContainer;
	FrameEventListenerContainer frameEventListeners_;
};
