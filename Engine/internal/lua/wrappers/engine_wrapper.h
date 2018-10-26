// Warning: this file is generated by wrapper.py.

#pragma once

#include "engine.h"

#include "lua++.h"
#include "tools/string.h"

class Engine_Wrapper {
	static int EngineInstance(lua_State* L) {
		return Lua::reference<Engine>(L);
	}

	static int ToString(lua_State* L) {
		Engine* _p = Engine::instance();
		lua_pushstring(L, String::Format("Engine@0x%p", _p).c_str());
		return 1;
	}

	// bool Startup(uint width, uint height)
	static int Startup(lua_State* L) {
		Engine* _p = Engine::instance();
		uint height = Lua::get<uint>(L, 3);
		uint width = Lua::get<uint>(L, 2);
		return Lua::push(L, _p->Startup(width, height));
	}

	// void Shutdown()
	static int Shutdown(lua_State* L) {
		Engine* _p = Engine::instance();
		_p->Shutdown();
		return 0;
	}

	// void Update()
	static int Update(lua_State* L) {
		Engine* _p = Engine::instance();
		_p->Update();
		return 0;
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<Engine>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& regs) {
		regs.push_back(luaL_Reg { "EngineInstance", EngineInstance });

		luaL_Reg metalib[] = {
			{ "Startup", Startup },
			{ "Shutdown", Shutdown },
			{ "Update", Update },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<Engine>(L, metalib, nullptr);
	}
};
