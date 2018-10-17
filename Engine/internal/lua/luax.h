#pragma once

#include <string>
#include <vector>
#include <cassert>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

namespace LuaPrivate {

static int _panic(lua_State* L) {
	Debug::LogError("onPanic: %s", lua_tostring(L, -1));

	// TODO: lua will call abort() anyway.
	return 0;
}

static void _error(lua_State *L) {
	Debug::LogError("onError: %s", lua_tostring(L, -1));
}

static int _print(lua_State *L) {
	std::string message;
	int nargs = lua_gettop(L);

	for (int i = 1; i <= nargs; i++) {
		if (i != 1) { message = message + " "; }

		switch (lua_type(L, i)) {
			case LUA_TNIL:
				message += "nil";
				break;
			case LUA_TBOOLEAN:
				message += lua_toboolean(L, i) != 0 ? "true" : "false";
				break;
			case LUA_TLIGHTUSERDATA:
				message += "lightuserdata";
				break;
			case LUA_TNUMBER:
				message += std::to_string(lua_tonumber(L, i));
				break;
			case LUA_TSTRING:
				message += lua_tostring(L, i);
				break;
			case LUA_TTABLE:
				message += "table";
				break;
			case LUA_TFUNCTION:
				message += "function";
				break;
			case LUA_TUSERDATA:
				message += "userdata";
				break;
			case LUA_TTHREAD:
				message += "thread";
				break;
		}
	}

	Debug::Log(message.c_str());

	return 0;
}

}	// namespace LuaPrivate

namespace Lua {

static void initialize(lua_State* L, luaL_Reg* libs, const char* entry) {
	std::vector<luaL_Reg> regs {
		{ "base", luaopen_base },
		{ "io", luaopen_io, },
	};

	for (; libs->func != nullptr; ++libs) {
		regs.push_back(*libs);
	}

	luaL_openlibs(L);

	for (luaL_Reg& reg : regs) {
		luaL_requiref(L, reg.name, reg.func, 1);
		lua_pop(L, 1);
	}

	lua_atpanic(L, LuaPrivate::_panic);

	// register print
	luaL_Reg printlib[] = {
		{ "print", LuaPrivate::_print },
		{ nullptr, nullptr }
	};

	lua_getglobal(L, "_G");
	luaL_setfuncs(L, printlib, 0);
	lua_pop(L, 1);
	//

	if (luaL_dofile(L, entry) != 0) {
		LuaPrivate::_error(L);
	}
}

template <class T>
static const char* metatableName() {
	return typeid(T).name();
}

template <class T>
static T* userdataPtr(lua_State* L, int index, const char* metatable) {
	//T** p = (T**)luaL_checkudata(L, 1, Lua::metatableName<T>());
	T** p = (T**)lua_touserdata(L, 1);
	if (p == nullptr) { return nullptr; }

	return *p;
}

template <class T>
static T* userdataSharedPtr(lua_State* L, int index, const char* metatable) {
	//T* p = (T*)luaL_checkudata(L, index, metatable);
	T* p = (T*)lua_touserdata(L, index);
	return p;
}

template <class T>
static T* callerSharedPtr(lua_State* L, int nargs) {
	assert(lua_gettop(L) == nargs + 1 && "invalid function call");
	return userdataSharedPtr<T>(L, 1, metatableName<T>());
}

template <class T>
static T* callerPtr(lua_State* L, int nargs) {
	assert(lua_gettop(L) == nargs + 1 && "invalid function call");
	return userdataPtr<T>(L, 1, metatableName<T>());
}

template <class T>
static int copyUserdata(lua_State* L, const T& value) {
	new(lua_newuserdata(L, sizeof(T))) T(value);

	luaL_getmetatable(L, metatableName<T>());
	lua_setmetatable(L, -2);
	return 1;
}

template <class T>
static int push(lua_State* L, const T& value) {
	return copyUserdata<T>(L, value);
}

template <>
static int push(lua_State* L, const bool& value) {
	lua_pushboolean(L, value);
	return 1;
}

template <>
static int push(lua_State* L, const int& value) {
	lua_pushinteger(L, value);
	return 1;
}

template <>
static int push(lua_State* L, const unsigned& value) {
	lua_pushinteger(L, value);
	return 1;
}

template <>
static int push(lua_State* L, const long& value) {
	lua_pushinteger(L, value);
	return 1;
}

template <>
static int push(lua_State* L, const unsigned long& value) {
	lua_pushinteger(L, value);
	return 1;
}

template <>
static int push(lua_State* L, const long long& value) {
	lua_pushinteger(L, value);
	return 1;
}

template <>
static int push(lua_State* L, const unsigned long long& value) {
	lua_pushinteger(L, value);
	return 1;
}

template <>
static int push(lua_State* L, const float& value) {
	lua_pushnumber(L, value);
	return 1;
}

template <>
static int push(lua_State* L, const double& value) {
	lua_pushnumber(L, value);
	return 1;
}

template <>
static int push(lua_State* L, const std::string& value) {
	lua_pushstring(L, value.c_str());
	return 1;
}

template <class T>
static int pushList(lua_State* L, const T& container) {
	lua_newtable(L);
	for (int i = 0; i < container.size(); ++i) {
		push(L, container[i]);
		lua_rawseti(L, -2, i + 1);
	}

	return 1;
}

template <class T>
static T get(lua_State* L, int index) {
	return *userdataSharedPtr<T>(L, -1, metatableName<T>());
}

template <>
static std::string get(lua_State* L, int index) {
	luaL_checktype(L, index, LUA_TSTRING);
	return lua_tostring(L, index);
}

template <>
static bool get(lua_State* L, int index) {
	luaL_checktype(L, index, LUA_TBOOLEAN);
	return !!lua_toboolean(L, index);
}

template <>
static int get(lua_State* L, int index) {
	luaL_checktype(L, index, LUA_TNUMBER);
	return (int)lua_tointeger(L, index);
}

template <>
static unsigned get(lua_State* L, int index) {
	luaL_checktype(L, index, LUA_TNUMBER);
	return (unsigned)lua_tointeger(L, index);
}

template <>
static long get(lua_State* L, int index) {
	luaL_checktype(L, index, LUA_TNUMBER);
	return (long)lua_tointeger(L, index);
}

template <>
static unsigned long get(lua_State* L, int index) {
	luaL_checktype(L, index, LUA_TNUMBER);
	return (unsigned long)lua_tointeger(L, index);
}

template <>
static long long get(lua_State* L, int index) {
	luaL_checktype(L, index, LUA_TNUMBER);
	return (long long)lua_tointeger(L, index);
}

template <>
static unsigned long long get(lua_State* L, int index) {
	luaL_checktype(L, index, LUA_TNUMBER);
	return (unsigned long long)lua_tointeger(L, index);
}

template <>
static float get(lua_State* L, int index) {
	luaL_checktype(L, index, LUA_TNUMBER);
	return (float)lua_tonumber(L, index);
}

template <>
static double get(lua_State* L, int index) {
	luaL_checktype(L, index, LUA_TNUMBER);
	return lua_tonumber(L, index);
}

template <class T>
static std::vector<T> getList(lua_State* L, int index) {
	luaL_checktype(L, index, LUA_TTABLE);
	int size = (int)lua_rawlen(L, index);
	std::vector<T> container;

	for (int i = 1; i <= size; ++i) {
		lua_pushinteger(L, i);
		lua_gettable(L, -2);
		if (lua_isnil(L, -1)) {
			size = i - 1;
			break;
		}

		container.push_back(get<T>(L, -1));
		lua_pop(L, 1);
	}

	return container;
}

template <class T>
static int fromShared(lua_State* L, T ptr) {
	return copyUserdata(L, ptr);
}

template <class T>
static int deleteSharedPtr(lua_State* L) {
	T* ptr = callerSharedPtr<T>(L, 0);
	if (ptr != nullptr) { ptr->reset(); }
	return 0;
}

template <class T>
static int newObject(lua_State* L) {
	T** memory = (T**)lua_newuserdata(L, sizeof(T*));
	*memory = new T;

	luaL_getmetatable(L, Lua::metatableName<T>());
	lua_setmetatable(L, -2);
	return 1;
}

template <class T>
static int newInterface(lua_State* L) {
	T** memory = (T**)lua_newuserdata(L, sizeof(T*));
	*memory = new T;

	luaL_getmetatable(L, Lua::metatableName<T::Interface>());
	lua_setmetatable(L, -2);
	return 1;
}

template <class T>
static int deletePtr(lua_State* L) {
	delete callerPtr<T>(L, 0);
	return 0;
}

template <class T>
static int reference(lua_State* L) {
	T** memory = (T**)lua_newuserdata(L, sizeof(T*));
	*memory = T::instance();

	luaL_getmetatable(L, Lua::metatableName<T>());
	lua_setmetatable(L, -2);

	return 1;
}

template <class T>
static void createMetatable(lua_State* L) {
	luaL_newmetatable(L, metatableName<T>());

	// duplicate metatable.
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	lua_pop(L, 1);
}

template <class T>
static void initMetatable(lua_State* L, luaL_Reg* lib, const char* baseClass) {
	luaL_getmetatable(L, metatableName<T>());
	luaL_setfuncs(L, lib, 0);

	if (baseClass != nullptr) {
		luaL_getmetatable(L, baseClass);
		lua_setmetatable(L, -2);
	}

	lua_pop(L, 1);
}

}	// namespace Lua
