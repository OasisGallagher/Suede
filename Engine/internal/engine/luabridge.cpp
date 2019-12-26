#include "luabridge.h"

#include "profiler.h"
#include "internal/base/context.h"
#include "../lua/wrappers/luaconfig.h"

LuaBridge::LuaBridge(Context* context) : updateRef_(LUA_NOREF) {
	luaL_Reg lualibs[] = {
		{ "Suede", Lua::configure },
		{ nullptr, nullptr }
	};

	L = luaL_newstate();
	Lua::initialize(L, lualibs, "resources/lua/main.lua");
	Lua::invokeGlobalFunction(L, "SuedeGlobal.Awake");
}

LuaBridge::~LuaBridge() {
	lua_close(L);
}

void LuaBridge::Update(float deltaTime) {
	if (updateRef_ == LUA_NOREF) {
		Lua::invokeGlobalFunction(L, "SuedeGlobal.Start");
		updateRef_ = Lua::getGlobalFunctionRef(L, "SuedeGlobal.Update");
	}

	// Disable lua supports.
	uint64 start = Profiler::GetTimeStamp();
	Lua::invokeGlobalFunction(L, updateRef_);
	context_->GetStatistics()->SetScriptElapsed(
		Profiler::TimeStampToSeconds(Profiler::GetTimeStamp() - start)
	);
}
