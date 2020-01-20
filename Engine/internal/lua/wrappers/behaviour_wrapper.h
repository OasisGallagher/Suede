// Warning: this file is generated by wrapper.py.

#pragma once

#include "behaviour.h"

#include "lua++.h"
#include "tools/string.h"

class Behaviour_Wrapper {
	static int ToString(lua_State* L) {
		Behaviour* _p = Lua::callerPtr<Behaviour>(L);

		lua_pushstring(L, String::Format("Behaviour@0x%p", _p).c_str());
		return 1;
	}

	// virtual void Awake()
	static int Awake(lua_State* L) {
		Behaviour* _p = Lua::callerPtr<Behaviour>(L);
		_p->Awake();
		return 0;
	}

	// virtual void Update()
	static int Update(lua_State* L) {
		Behaviour* _p = Lua::callerPtr<Behaviour>(L);
		_p->Update();
		return 0;
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<Behaviour>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		luaL_Reg metalib[] = {
			{ "__gc", Lua::deletePtr<Behaviour> },
			{ "__tostring", ToString }, 
			{ "Awake", Awake },
			{ "Update", Update },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<Behaviour>(L, metalib, TypeID<Component>::string());
	}
};