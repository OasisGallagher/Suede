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

Engine::Engine() : Singleton2<Engine>(MEMORY_NEW(EngineInternal), Memory::DeleteRaw<EngineInternal>) {}

bool Engine::Startup(uint width, uint height) { return _suede_dptr()->Startup(width, height); }
void Engine::Shutdown() { _suede_dptr()->Shutdown(); }
void Engine::Update() { _suede_dptr()->Update(); }
void Engine::AddFrameEventListener(FrameEventListener* listener) { _suede_dptr()->AddFrameEventListener(listener); }
void Engine::RemoveFrameEventListener(FrameEventListener* listener) { _suede_dptr()->RemoveFrameEventListener(listener); }

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

	// create profiler first to ensure it'll be destroyed last.
	Profiler::instance();

	Screen::instance()->Resize(width, height);
	World::instance()->Initialize();

	luaL_Reg lualibs[] = {
		{ "Suede", Lua::configure },
		{ nullptr, nullptr }
	};

	L = luaL_newstate();
	Lua::initialize(L, lualibs, "resources/lua/main.lua");
	InvokeLuaMethod("Awake");

	return true;
}

void EngineInternal::Shutdown() {
	World::instance()->Finalize();
	lua_close(L);
	L = nullptr;
}

void EngineInternal::Update() {
	InvokeLuaMethod("Update");

	SortFrameEventListeners();
	ForEachFrameEventListener(frameEventListeners_, &FrameEventListener::OnFrameEnter);
	World::instance()->RenderingUpdate();
	ForEachFrameEventListener(frameEventListeners_, &FrameEventListener::OnFrameLeave);
}

void EngineInternal::InvokeLuaMethod(const char* name) {
	lua_getglobal(L, "SuedeGlobal");
	lua_getfield(L, -1, name);
	int n = lua_pcall(L, 0, 0, 0);
	if (n != LUA_OK) {
		Debug::LogError("invoke \"%s\" failed(%d): %s.", name, n, lua_tostring(L, -1));
		lua_pop(L, 1);
	}

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
