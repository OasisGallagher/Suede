#ifdef _DEBUG
#include "vld/vld.h"
#endif

#include <vector>
#include <algorithm>

#include <ZThread/Config.h>

#include "glef.h"
#include "time2.h"
#include "world.h"
#include "driver.h"
#include "api/gl.h"
#include "engine.h"
#include "screen.h"
#include "statistics.h"
#include "graphicscanvas.h"
#include "debug/profiler.h"

typedef std::vector<FrameEventListener*> FrameEventListenerContainer;
static FrameEventListenerContainer frameEventListeners_;

template <class MemFunc>
static void ForEachFrameEventListener(MemFunc func) {
	for (FrameEventListenerContainer::iterator ite = frameEventListeners_.begin();
		ite != frameEventListeners_.end(); ++ite) {
		((*ite)->*func)();
	}
}

static void SortFrameEventListeners() {
	struct FrameEventComparer {
		bool operator()(FrameEventListener* lhs, FrameEventListener* rhs) {
			return lhs->GetFrameEventQueue() < rhs->GetFrameEventQueue();
		}
	};

	static FrameEventComparer comparer;
	std::sort(frameEventListeners_.begin(), frameEventListeners_.end(), comparer);
}

static void SetDefaultGLStates() {
	GL::ClearDepth(1);
	GL::DepthRange(0, 1);

	GL::Enable(GL_DEPTH_TEST);
	GL::DepthFunc(GL_LEQUAL);

	GL::Enable(GL_CULL_FACE);
	GL::CullFace(GL_BACK);
	
	GL::DepthMask(GL_TRUE);
}

static void OnZThreadException(const std::exception& exception) {
	Debug::Output("!!! Thread Exception %s\n", exception.what());
	throw exception;
}

static void OnTerminate() {
	Debug::Break();
}

bool Engine::Initialize() {
	setlocale(LC_ALL, "");

	std::set_terminate(OnTerminate);
	ZThread::ztException = OnZThreadException;

	Debug::Initialize();

	if (!Driver::Initialize()) {
		return false;
	}

	SetDefaultGLStates();

	GLEF::Initialize();
	Profiler::Initialize();

	return true;
}

void Engine::Release() {
	WorldInstance().reset();
}

void Engine::AddFrameEventListener(FrameEventListener* listener) {
	if (std::find(frameEventListeners_.begin(), frameEventListeners_.end(), listener) == frameEventListeners_.end()) {
		frameEventListeners_.push_back(listener);
	}
}

void Engine::RemoveFrameEventListener(FrameEventListener* listener) {
	std::vector<FrameEventListener*>::iterator ite = std::find(frameEventListeners_.begin(), frameEventListeners_.end(), listener);
	if (ite != frameEventListeners_.end()) {
		frameEventListeners_.erase(ite);
	}
}

void Engine::Update() {
	Time::Update();
	Statistics::Update();

	SortFrameEventListeners();
	ForEachFrameEventListener(&FrameEventListener::OnFrameEnter);
	WorldInstance()->Update();
	ForEachFrameEventListener(&FrameEventListener::OnFrameLeave);
}
