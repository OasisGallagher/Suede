#pragma once
#include <vector>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

typedef LUA_NUMBER LuaNumber;
class LuaState {
public:

public:
	void SetState(lua_State* value) { L = value; }

	// Any string that lua_tostring returns always has a zero at its end, 
	// but it can have other zeros inside it. The lua_strlen function returns the correct length of the string.
	std::string GetString() {
		luaL_checktype(L, -1, LUA_TSTRING);
		return lua_tostring(L, -1);
	}

	void SetString(const std::string& value) {
		lua_settop(L, 0);
		lua_pushstring(L, value.c_str());
	}

	LuaNumber GetNumber() {
		luaL_checktype(L, -1, LUA_TNUMBER);
		return lua_tonumber(L, -1);
	}

	void SetNumber(LuaNumber value) {
		lua_settop(L, 0);
		lua_pushnumber(L, value);
	}

	void DumpStack() {
		int top = lua_gettop(L);
		for (int i = 1; i <= top; i++) {  /* repeat for each level */
			int t = lua_type(L, i);
			switch (t) {
				case LUA_TSTRING:  /* strings */
					printf("`%s'", lua_tostring(L, i));
					break;

				case LUA_TBOOLEAN:  /* booleans */
					printf(lua_toboolean(L, i) ? "true" : "false");
					break;

				case LUA_TNUMBER:  /* numbers */
					printf("%g", lua_tonumber(L, i));
					break;

				default:  /* other values */
					printf("%s", lua_typename(L, t));
					break;
			}

			printf("  ");  /* put a separator */
		}

		printf("\n");  /* end the listing */
	}

private:
	lua_State* L;
};
