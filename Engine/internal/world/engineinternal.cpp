#ifdef _DEBUG
#include "vld/vld.h"
#endif

#include "glef.h"
#include "opengldriver.h"
#include "engineinternal.h"
#include "builtinproperties.h"

#include "worldinternal.h"
#include "../tools/time2internal.h"
#include "../tools/screeninternal.h"
#include "../tools/profilerinternal.h"
#include "../tools/tagmanagerinternal.h"
#include "../tools/statisticsinternal.h"

#include "../lua/wrappers/luaconfig.h"

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

void EngineInternal::SortFrameEventListeners() {
	struct FrameEventComparer {
		bool operator()(FrameEventListener* lhs, FrameEventListener* rhs) const {
			return lhs->GetFrameEventQueue() < rhs->GetFrameEventQueue();
		}
	};

	static FrameEventComparer comparer;
	std::sort(frameEventListeners_.begin(), frameEventListeners_.end(), comparer);
}

bool EngineInternal::Startup(uint width, uint height) {
	setlocale(LC_ALL, "");
	std::set_terminate(OnTerminate);
	ZThread::ztException = OnZThreadException;

	if (!Debug::Initialize()) { return false; }
	if (!OpenGLDriver::Load()) { return false; }

	if (!GLEF::instance()->Load("resources/data/GLEF.dat")) {
		return false;
	}

	Time::implement(new TimeInternal);
	Profiler::implement(new ProfilerInternal);
	TagManager::implement(new TagManagerInternal);
	Statistics::implement(new StatisticsInternal);
	Screen::implement(new ScreenInternal(width, height));

	World::implement(new WorldInternal);
	World::instance()->Initialize();

	luaL_Reg lualibs[] = {
		{ "Suede", Lua::configure },
		{ nullptr, nullptr }
	};

	L = luaL_newstate();
	Lua::initialize(L, lualibs, "resources/lua/main.lua");

// 	for (;;) {
// 		lua_getfield(L, -1, "update");
// 		lua_call(L, 0, 0);
// 	}

	return true;
}

void EngineInternal::Shutdown() {
	World::instance()->Finalize();
	lua_close(L);
	L = nullptr;
}

void EngineInternal::Update() {
	LuaUpdate();

	SortFrameEventListeners();
	ForEachFrameEventListener(frameEventListeners_, &FrameEventListener::OnFrameEnter);
	World::instance()->RenderingUpdate();
	ForEachFrameEventListener(frameEventListeners_, &FrameEventListener::OnFrameLeave);
}

void EngineInternal::LuaUpdate() {
	lua_getglobal(L, "SuedeGlobal");
	lua_getfield(L, -1, "update");
	lua_call(L, 0, 0);
	lua_pop(L, 1);
}

void EngineInternal::AddFrameEventListener(FrameEventListener* listener) {
	if (std::find(frameEventListeners_.begin(), frameEventListeners_.end(), listener) == frameEventListeners_.end()) {
		frameEventListeners_.push_back(listener);
	}
}

void EngineInternal::RemoveFrameEventListener(FrameEventListener* listener) {
	std::vector<FrameEventListener*>::iterator ite = std::find(frameEventListeners_.begin(), frameEventListeners_.end(), listener);
	if (ite != frameEventListeners_.end()) {
		frameEventListeners_.erase(ite);
	}
}
