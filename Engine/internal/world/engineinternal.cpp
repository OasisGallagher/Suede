#ifdef _DEBUG
#include "vld/vld.h"
#endif

#include <ZThread/Thread.h>

#include "glef.h"
#include "physics.h"
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

Engine::Engine() : singleton2<Engine>(MEMORY_NEW(EngineInternal), Memory::DeleteRaw<EngineInternal>) {}

bool Engine::Startup(uint width, uint height) { return _suede_dinstance()->Startup(width, height); }
void Engine::Shutdown() { _suede_dinstance()->Shutdown(); }
void Engine::Update() { _suede_dinstance()->Update(); }
void Engine::AddFrameEventListener(FrameEventListener* listener) { _suede_dinstance()->AddFrameEventListener(listener); }
void Engine::RemoveFrameEventListener(FrameEventListener* listener) { _suede_dinstance()->RemoveFrameEventListener(listener); }

static void OnTerminate() {
	Debug::Break();
}

static void OnZThreadException(const std::exception& exception) {
	Debug::Output("!!! Thread Exception %s\n", exception.what());
	throw exception;
}

EngineInternal::EngineInternal() : updateRef_(LUA_NOREF) {

}

template <class... Args>
bool EngineInternal::InvokeCurrentLuaMethod(Args... args) {
	Lua::push(L, args...);
	int r = lua_pcall(L, sizeof...(Args), 0, 0);
	if (r != LUA_OK) {
		Debug::LogError("invoke function failed(%d): %s.", r, lua_tostring(L, -1));
		lua_pop(L, 1);
		return false;
	}

	return true;
}

bool EngineInternal::Startup(uint width, uint height) {
	setlocale(LC_ALL, "");
	std::set_terminate(OnTerminate);
	ZThread::ztException = OnZThreadException;
	ZThread::Thread::markMainThread();

	if (!Debug::Initialize()) { return false; }
	if (!OpenGLDriver::Load()) { return false; }

	if (!GLEF::instance()->Load("resources/data/GLEF.dat")) {
		return false;
	}

	// create profiler first to ensure it's destroyed last.
	Profiler::TimeStampToSeconds(0);

	Screen::Resize(width, height);
	World::Initialize();

	//Physics::SetDebugDrawEnabled(true);
	Physics::SetGravity(glm::vec3(0, -9.8f, 0));

	luaL_Reg lualibs[] = {
		{ "Suede", Lua::configure },
		{ nullptr, nullptr }
	};

	L = luaL_newstate();
	Lua::initialize(L, lualibs, "resources/lua/main.lua");
	Lua::invokeGlobalFunction(L, "SuedeGlobal.Awake");

	return true;
}

void EngineInternal::Shutdown() {
	World::Finalize();
	lua_close(L);
	L = nullptr;
}

void EngineInternal::Update() {
	uint64 start = Profiler::GetTimeStamp();
	if (updateRef_ == LUA_NOREF) {
		Lua::invokeGlobalFunction(L, "SuedeGlobal.Start");
		updateRef_ = Lua::getGlobalFunctionRef(L, "SuedeGlobal.Update");
	}

	Lua::invokeGlobalFunction(L, updateRef_);
	uint64 now = Profiler::GetTimeStamp();
	Statistics::SetScriptElapsed(
		Profiler::TimeStampToSeconds(now - start)
	);

	start = now;
	FrameEventListenerContainer cont(listeners_);
	std::for_each(cont.begin(), cont.end(), std::mem_fun(&FrameEventListener::OnFrameEnter));
	now = Profiler::GetTimeStamp();
	double frameEnter = Profiler::TimeStampToSeconds(now - start);
	
	World::Update();

	start = now;
	std::for_each(cont.begin(), cont.end(), std::mem_fun(&FrameEventListener::OnFrameLeave));
	now = Profiler::GetTimeStamp();
	double frameLeave = Profiler::TimeStampToSeconds(now - start);

	Debug::Output("frameEnter: %.2f, frameLeave: %.2f", frameEnter * 1000, frameLeave * 1000);
}

void EngineInternal::AddFrameEventListener(FrameEventListener* listener) {
	struct FrameEventComparer {
		bool operator()(FrameEventListener* lhs, FrameEventListener* rhs) const {
			return lhs->GetFrameEventQueue() < rhs->GetFrameEventQueue();
		}
	};

	auto pos = std::upper_bound(listeners_.begin(), listeners_.end(), listener, FrameEventComparer());
	listeners_.insert(pos, listener);
}

void EngineInternal::RemoveFrameEventListener(FrameEventListener* listener) {
	std::vector<FrameEventListener*>::iterator ite = std::find(listeners_.begin(), listeners_.end(), listener);
	if (ite != listeners_.end()) {
		listeners_.erase(ite);
	}
}
