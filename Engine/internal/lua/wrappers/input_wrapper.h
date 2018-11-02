// Warning: this file is generated by wrapper.py.

#pragma once

#include "input.h"

#include "lua++.h"
#include "tools/string.h"

class Input_Wrapper {
	static int ToString(lua_State* L) {
		Input* _p = Lua::callerPtr<Input>(L);

		lua_pushstring(L, String::Format("Input@0x%p", _p).c_str());
		return 1;
	}

	static int ToStringStatic(lua_State* L) {
		lua_pushstring(L, "static Input");
		return 1;
	}

	static int InputStatic(lua_State* L) {
		lua_newtable(L);

		luaL_Reg funcs[] = {
			{ "GetKey", GetKey },
			{ "GetKeyUp", GetKeyUp },
			{ "GetKeyDown", GetKeyDown },
			{ "GetMouseButton", GetMouseButton },
			{ "GetMouseButtonUp", GetMouseButtonUp },
			{ "GetMouseButtonDown", GetMouseButtonDown },
			{ "GetMouseWheelDelta", GetMouseWheelDelta },
			{ "GetMousePosition", GetMousePosition },
			{"__tostring", ToStringStatic },
			{ nullptr, nullptr }
		};

		luaL_setfuncs(L, funcs, 0);

		return 1;
	}
	// static bool GetKey(KeyCode key)
	static int GetKey(lua_State* L) {
		KeyCode key = Lua::get<KeyCode>(L, 1);
		
		return Lua::push(L, Input::GetKey(key));
	}

	// static bool GetKeyUp(KeyCode key)
	static int GetKeyUp(lua_State* L) {
		KeyCode key = Lua::get<KeyCode>(L, 1);
		
		return Lua::push(L, Input::GetKeyUp(key));
	}

	// static bool GetKeyDown(KeyCode key)
	static int GetKeyDown(lua_State* L) {
		KeyCode key = Lua::get<KeyCode>(L, 1);
		
		return Lua::push(L, Input::GetKeyDown(key));
	}

	// static bool GetMouseButton(int button)
	static int GetMouseButton(lua_State* L) {
		int button = Lua::get<int>(L, 1);
		
		return Lua::push(L, Input::GetMouseButton(button));
	}

	// static bool GetMouseButtonUp(int button)
	static int GetMouseButtonUp(lua_State* L) {
		int button = Lua::get<int>(L, 1);
		
		return Lua::push(L, Input::GetMouseButtonUp(button));
	}

	// static bool GetMouseButtonDown(int button)
	static int GetMouseButtonDown(lua_State* L) {
		int button = Lua::get<int>(L, 1);
		
		return Lua::push(L, Input::GetMouseButtonDown(button));
	}

	// static float GetMouseWheelDelta()
	static int GetMouseWheelDelta(lua_State* L) {
		return Lua::push(L, Input::GetMouseWheelDelta());
	}

	// static glm::ivec2 GetMousePosition()
	static int GetMousePosition(lua_State* L) {
		return Lua::push(L, Input::GetMousePosition());
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<Input>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		fields.push_back(luaL_Reg{ "Input", InputStatic });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deletePtr<Input> },
			{ "__tostring", ToString }, 
			{ nullptr, nullptr }
		};

		Lua::initMetatable<Input>(L, metalib, nullptr);
	}
};
