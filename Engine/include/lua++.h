#pragma once

#include <string>
#include <vector>
#include <memory>

#include "tools/math2.h"
#include "debug/debug.h"

extern "C" {
#include "lua/lua.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"
}

namespace Lua {
static int _panic(lua_State* L) {
	Debug::LogError("onPanic: %s", lua_tostring(L, -1));
	lua_pop(L, 1);

	// SUEDE TODO: lua will call abort() anyway.
	return 0;
}

static void _error(lua_State *L) {
	Debug::LogError("onError: %s", lua_tostring(L, -1));
	lua_pop(L, 1);
}

static void _extractMsg(lua_State* L, std::string& message, int i) {
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

static int _print(lua_State *L) {
	std::string message;
	int nargs = lua_gettop(L);

	for (int i = 1; i <= nargs; i++) {
		if (i != 1) { message += " "; }
		_extractMsg(L, message, i);
	}

	Debug::Log(message.c_str());

	return 0;
}

static void _registerGlobals(lua_State* L) {
	luaL_Reg globals[] = {
		{ "print", _print },
		{ nullptr, nullptr }
	};

	lua_getglobal(L, "_G");
	luaL_setfuncs(L, globals, 0);
	lua_pop(L, 1);
}

template <class T>
class _MetatableID {
	static const int dummy = 0;

public:
	static intptr_t value() { return (intptr_t)&dummy; }
};

template <class R, class... Args>
class _FBase {
public:
	_FBase(lua_State* L) : L(L) {
		ref_ = luaL_ref(L, LUA_REGISTRYINDEX);
	}

	virtual ~_FBase() {
		luaL_unref(L, LUA_REGISTRYINDEX, ref_);
	}

	R operator()(Args... args) {
		lua_rawgeti(L, LUA_REGISTRYINDEX, ref_);

		lua_pushvalue(L, -1);
		Lua::push(L, args...);

		R _r();
		if (lua_pcall(L, sizeof...(Args), 1, 0) != 0) {
			_error(L);
		}
		else {
			r = Lua::get<R>(L, -1);
			lua_pop(L, 1);
		}

		ref_ = luaL_ref(L, LUA_REGISTRYINDEX);
		return _r;
	}

protected:
	int ref_;
	lua_State* L;
};

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

	lua_atpanic(L, _panic);

	_registerGlobals(L);
	
	if (luaL_dofile(L, entry) != 0) {
		_error(L);
	}
}

template <class T>
static const char* metatableName() {
	//return typeid(T).name();
	static std::string str = std::to_string(_MetatableID<T>::value());
	return str.c_str();
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
	if (lua_gettop(L) != nargs + 1) {
		Debug::LogError("invalid function call");
		return nullptr;
	}

	return userdataSharedPtr<T>(L, 1, metatableName<T>());
}

template <class T>
static T* callerPtr(lua_State* L, int nargs) {
	if (lua_gettop(L) != nargs + 1) {
		Debug::LogError("invalid function call");
		return nullptr;
	}

	return userdataPtr<T>(L, 1, metatableName<T>());
}

template <class T>
static int copyUserdata(lua_State* L, const T& value) {
	new(lua_newuserdata(L, sizeof(T))) T(value);

	luaL_getmetatable(L, metatableName<T>());
	lua_setmetatable(L, -2);
	return 1;
}

static int push(lua_State* L) {
	return 0;
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

template <class T, class... R>
static int push(lua_State* L, T arg, R... args) {
	int n = push(L, arg);
	n += push(L, args...);
	return n;
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
static T _glmConvert(lua_State* L, int index) {
	T ans;
	float* ptr = (float*)&ans;
	std::vector<float> values = getList<float>(L, index);
	for (int i = 0; i < Math::Min(sizeof(T) / sizeof(float), values.size()); ++i) {
		*ptr++ = values[i];
	}

	return ans;
}

template <>
static glm::vec2 get(lua_State* L, int index) {
	return _glmConvert<glm::vec2>(L, index);
}

template <>
static glm::vec3 get(lua_State* L, int index) {
	return _glmConvert<glm::vec3>(L, index);
}

template <>
static glm::vec4 get(lua_State* L, int index) {
	return _glmConvert<glm::vec4>(L, index);
}

template <>
static glm::quat get(lua_State* L, int index) {
	return _glmConvert<glm::quat>(L, index);
}

template <>
static glm::mat2 get(lua_State* L, int index) {
	return _glmConvert<glm::mat2>(L, index);
}

template <>
static glm::mat3 get(lua_State* L, int index) {
	return _glmConvert<glm::mat3>(L, index);
}

template <>
static glm::mat4 get(lua_State* L, int index) {
	return _glmConvert<glm::mat4>(L, index);
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

template <class R, class... Args>
class IFunction : public _FBase <R, Args...> {
public:
	IFunction(lua_State* L) : FBase(L) {}
};

template <class... Args>
class IFunction<void, Args...> : public _FBase<void, Args...> {
public:
	IFunction(lua_State* L) : _FBase(L) {}

public:
	void operator()(Args... args) {
		lua_rawgeti(L, LUA_REGISTRYINDEX, ref_);

		lua_pushvalue(L, -1);
		Lua::push(L, args...);
		if (lua_pcall(L, sizeof...(Args), 0, 0) != 0) {
			_error(L);
		}

		ref_ = luaL_ref(L, LUA_REGISTRYINDEX);
	}
};

template <class R, class... Args>
using Func = std::shared_ptr<IFunction<R, Args...>>;

template <class R, class... Args>
Func<R, Args...> make_func(lua_State* L) {
	if (lua_type(L, -1) != LUA_TFUNCTION) {
		Debug::LogError("top of stack must be a function");
		return nullptr;
	}

	return std::make_shared<Func<R, Args...>::element_type>(L);
}

}	// namespace Lua
