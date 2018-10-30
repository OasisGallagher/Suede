// Warning: this file is generated by wrapper.py.

#pragma once

#include "polygon.h"

#include "lua++.h"
#include "tools/string.h"

class Polygon_Wrapper {
	static int NewPolygon(lua_State* L) {
		return Lua::newObject<Polygon>(L);
	}

	static int ToString(lua_State* L) {
		Polygon* _p = Lua::callerPtr<Polygon>(L);

		lua_pushstring(L, String::Format("Polygon@0x%p", _p).c_str());
		return 1;
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<Polygon>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		funcs.push_back(luaL_Reg { "NewPolygon", NewPolygon });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deletePtr<Polygon> },
			{ "__tostring", ToString }, 
			{ nullptr, nullptr }
		};

		Lua::initMetatable<Polygon>(L, metalib, nullptr);
	}
};

class Triangle_Wrapper {
	static int NewTriangle(lua_State* L) {
		return Lua::newObject<Triangle>(L);
	}

	static int ToString(lua_State* L) {
		Triangle* _p = Lua::callerPtr<Triangle>(L);

		lua_pushstring(L, String::Format("Triangle@0x%p", _p).c_str());
		return 1;
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<Triangle>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		funcs.push_back(luaL_Reg { "NewTriangle", NewTriangle });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deletePtr<Triangle> },
			{ "__tostring", ToString }, 
			{ nullptr, nullptr }
		};

		Lua::initMetatable<Triangle>(L, metalib, TypeID<Polygon>::string());
	}
};
