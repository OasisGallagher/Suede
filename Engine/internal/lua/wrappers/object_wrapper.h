// Warning: this file is generated by wrapper.py.

#pragma once

#include "lua++.h"
#include "object.h"

class Object_Wrapper {
	static int Clone(lua_State* L) {
		Object& _p = *Lua::callerSharedPtr<Object>(L, 0);
		return Lua::push(L, _p->Clone());
	}

	static int GetObjectType(lua_State* L) {
		Object& _p = *Lua::callerSharedPtr<Object>(L, 0);
		return Lua::push(L, _p->GetObjectType());
	}

	static int GetInstanceID(lua_State* L) {
		Object& _p = *Lua::callerSharedPtr<Object>(L, 0);
		return Lua::push(L, _p->GetInstanceID());
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<Object>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& regs) {
		luaL_Reg metalib[] = {
			{ "__gc", Lua::deleteSharedPtr<Object> },
			{ "Clone", Clone },
			{ "GetObjectType", GetObjectType },
			{ "GetInstanceID", GetInstanceID },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<Object>(L, metalib, nullptr);
	}
};
