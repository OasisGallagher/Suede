// Warning: this file is generated by wrapper.py.

#pragma once

#include "../luax.h"
#include "screen.h"

class Screen_Wrapper {
	static int ScreenInstance(lua_State* L) {
		return Lua::reference<Screen>(L);
	}

	static int GetWidth(lua_State* L) {
		Screen* _p = Screen::instance();
		return Lua::push(L, _p->GetWidth());
	}

	static int GetHeight(lua_State* L) {
		Screen* _p = Screen::instance();
		return Lua::push(L, _p->GetHeight());
	}

	static int Resize(lua_State* L) {
		Screen* _p = Screen::instance();
		uint width = Lua::get<uint>(L, -1);
		uint height = Lua::get<uint>(L, -2);
		_p->Resize(width, height);
		return 0;
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<Screen>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& regs) {
		regs.push_back(luaL_Reg { "ScreenInstance", ScreenInstance });

		luaL_Reg metalib[] = {
			{ "GetWidth", GetWidth },
			{ "GetHeight", GetHeight },
			{ "Resize", Resize },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<Screen>(L, metalib, nullptr);
	}
};