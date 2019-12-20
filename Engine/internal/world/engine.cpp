#ifdef _DEBUG
#include "vld/vld.h"
#endif

#include "engine.h"

#include <ZThread/Thread.h>

#include "world.h"
#include "screen.h"
#include "physics.h"
#include "profiler.h"

// Disable lua supports.
//#include "../lua/wrappers/luaconfig.h"

sorted_event<> Engine::frameEnter;
sorted_event<> Engine::frameLeave;

// Disable lua supports.
//static lua_State* L;
//static int updateRef_ = LUA_NOREF;

static void OnTerminate() {
	Debug::Break();
}

static void OnZThreadException(const std::exception& exception) {
	Debug::OutputToConsole("!!! Thread Exception %s\n", exception.what());
	throw exception;
}

bool Engine::Startup(uint width, uint height) {
	setlocale(LC_ALL, "");
	std::set_terminate(OnTerminate);
	ZThread::ztException = OnZThreadException;
	ZThread::Thread::markMainThread();

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

void Engine::Shutdown() {
	World::Finalize();

	// Disable lua supports.
	//lua_close(L);
	//L = nullptr;
}

void Engine::Update() {
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

	Engine::frameEnter.raise();
	World::Update();
	Engine::frameLeave.raise();
}

