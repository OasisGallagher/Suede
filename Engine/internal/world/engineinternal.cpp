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

// Disable lua supports.
//#include "../lua/wrappers/luaconfig.h"

sorted_event<> Engine::frameEnter;
sorted_event<> Engine::frameLeave;

Engine::Engine() : Singleton2<Engine>(new EngineInternal, t_delete<EngineInternal>) {}

bool Engine::Startup(uint width, uint height) { return _suede_dinstance()->Startup(width, height); }
void Engine::Shutdown() { _suede_dinstance()->Shutdown(); }
void Engine::Update() { _suede_dinstance()->Update(); }

static void OnTerminate() {
	Debug::Break();
}

static void OnZThreadException(const std::exception& exception) {
	Debug::Output("!!! Thread Exception %s\n", exception.what());
	throw exception;
}

// Disable lua supports.
EngineInternal::EngineInternal() /*: updateRef_(LUA_NOREF)*/ {

}

//template <class... Args>
//bool EngineInternal::InvokeCurrentLuaMethod(Args... args) {
//	Lua::push(L, args...);
//	int r = lua_pcall(L, sizeof...(Args), 0, 0);
//	if (r != LUA_OK) {
//		Debug::LogError("invoke function failed(%d): %s.", r, lua_tostring(L, -1));
//		lua_pop(L, 1);
//		return false;
//	}
//
//	return true;
//}

bool EngineInternal::Startup(uint width, uint height) {
	setlocale(LC_ALL, "");
	std::set_terminate(OnTerminate);
	ZThread::ztException = OnZThreadException;
	ZThread::Thread::markMainThread();

	if (!Debug::Initialize()) {
		return false;
	}

	if (!OpenGLDriver::Initialize()) {
		return false;
	}

	if (!Shader::LoadParser("resources/data/GLEF.dat")) {
		return false;
	}

	// create profiler first to ensure it's destroyed last.
	Profiler::TimeStampToSeconds(0);

	Screen::Resize(width, height);
	World::Initialize();

	//Physics::SetDebugDrawEnabled(true);
	Physics::SetGravity(Vector3(0, -9.8f, 0));

	// Disable lua supports.
	//luaL_Reg lualibs[] = {
	//	{ "Suede", Lua::configure },
	//	{ nullptr, nullptr }
	//};

	//L = luaL_newstate();
	//Lua::initialize(L, lualibs, "resources/lua/main.lua");
	//Lua::invokeGlobalFunction(L, "SuedeGlobal.Awake");

	return true;
}

void EngineInternal::Shutdown() {
	World::Finalize();
	// Disable lua supports.
	//lua_close(L);
	//L = nullptr;
}

void EngineInternal::Update() {
	// Disable lua supports.
	//if (updateRef_ == LUA_NOREF) {
	//	Lua::invokeGlobalFunction(L, "SuedeGlobal.Start");
	//	updateRef_ = Lua::getGlobalFunctionRef(L, "SuedeGlobal.Update");
	//}

	// Disable lua supports.
	//uint64 start = Profiler::GetTimeStamp();
	//Lua::invokeGlobalFunction(L, updateRef_);
	//Statistics::SetScriptElapsed(
	//	Profiler::TimeStampToSeconds(Profiler::GetTimeStamp() - start)
	//);

	Engine::frameEnter.fire();
	World::Update();
	Engine::frameLeave.fire();
}

