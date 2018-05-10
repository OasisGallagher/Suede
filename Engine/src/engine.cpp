#ifdef _DEBUG
#include "vld/vld.h"
#endif

#include <vector>

#include "glef.h"
#include "time2.h"
#include "world.h"
#include "driver.h"
#include "api/gl.h"
#include "engine.h"
#include "screen.h"
#include "resources.h"
#include "statistics.h"
#include "debug/debug.h"
#include "debug/profiler.h"

static std::vector<FrameEventListener*> frameEventListeners;
#define FOR_EACH_FRAME_EVENT_LISTENER(func)	for (uint i = 0; i < frameEventListeners.size(); ++i) frameEventListeners[i]->func()

static void SetDefaultGLStates() {
	GL::ClearDepth(1);
	GL::DepthRange(0, 1);

	GL::Enable(GL_DEPTH_TEST);
	GL::DepthFunc(GL_LEQUAL);

	GL::Enable(GL_CULL_FACE);
	GL::CullFace(GL_BACK);

	GL::DepthMask(GL_TRUE);
}

bool Engine::Initialize() {
	setlocale(LC_ALL, "");

	Debug::Initialize();

	if (!Driver::Initialize()) {
		return false;
	}

	SetDefaultGLStates();

	GLEF::Initialize();
	Profiler::Initialize();

	Resources::Import();

	return true;
}

void Engine::Release() {
	WorldInstance().reset();
}

void Engine::Resize(int w, int h) {
	Screen::Set(w, h);
	//GL::Viewport(0, 0, w, h);
}

void Engine::AddFrameEventListener(FrameEventListener* listener) {
	if (std::find(frameEventListeners.begin(), frameEventListeners.end(), listener) == frameEventListeners.end()) {
		frameEventListeners.push_back(listener);
	}
}

void Engine::RemoveFrameEventListener(FrameEventListener* listener) {
	std::vector<FrameEventListener*>::iterator ite = std::find(frameEventListeners.begin(), frameEventListeners.end(), listener);
	if (ite != frameEventListeners.end()) {
		frameEventListeners.erase(ite);
	}
}

void Engine::Update() {
	Time::Update();
	Statistics::Update();

	FOR_EACH_FRAME_EVENT_LISTENER(OnFrameEnter);
	WorldInstance()->Update();
	FOR_EACH_FRAME_EVENT_LISTENER(OnFrameLeave);
}
