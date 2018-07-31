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
#include "profiler.h"
#include "statistics.h"
#include "graphicscanvas.h"

static void OnTerminate() {
	Debug::Break();
}

static void OnZThreadException(const std::exception& exception) {
	Debug::Output("!!! Thread Exception %s\n", exception.what());
	throw exception;
}

template <class T, class MemFunc>
static void ForEachFrameEventListener(T& container, MemFunc func) {
	for (T::iterator ite = container.begin(); ite != container.end(); ++ite) {
		((*ite)->*func)();
	}
}

void Engine::SortFrameEventListeners() {
	struct FrameEventComparer {
		bool operator()(FrameEventListener* lhs, FrameEventListener* rhs) const {
			return lhs->GetFrameEventQueue() < rhs->GetFrameEventQueue();
		}
	};

	static FrameEventComparer comparer;
	std::sort(frameEventListeners_.begin(), frameEventListeners_.end(), comparer);
}

void Engine::SetDefaultGLStates() {
	GL::ClearDepth(1);
	GL::DepthRange(0, 1);

	GL::Enable(GL_DEPTH_TEST);
	GL::DepthFunc(GL_LEQUAL);

	GL::Enable(GL_CULL_FACE);
	GL::CullFace(GL_BACK);

	GL::DepthMask(GL_TRUE);

	GL::PolygonMode(GL_FRONT_AND_BACK, GL_LINE);
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

	return true;
}

void Engine::Release() {
	World::get()->Destroy();
	World::get().reset();
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
	SortFrameEventListeners();
	ForEachFrameEventListener(frameEventListeners_, &FrameEventListener::OnFrameEnter);
	World::get()->Update();
	ForEachFrameEventListener(frameEventListeners_, &FrameEventListener::OnFrameLeave);
}
