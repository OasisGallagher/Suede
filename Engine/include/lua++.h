#pragma once

#include <string>
#include <vector>
#include <memory>
#include <type_traits>

#include "color.h"

#include <glm/glm.hpp>

#include "tools/math2.h"
#include "debug/debug.h"
#include "tools/typeid.h"

extern "C" {
#include "lua/lua.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"
}

namespace Lua {

#pragma region internal helpers

inline int _panic(lua_State* L) {
	Debug::LogError("onPanic: %s", lua_tostring(L, -1));
	lua_pop(L, 1);

	// SUEDE TODO: lua will call abort() anyway.
	return 0;
}

inline void _error(lua_State *L) {
	Debug::LogError("onError: %s", lua_tostring(L, -1));
	lua_pop(L, 1);
}

static int _print(lua_State *L) {
	std::string message;
	int nargs = lua_gettop(L);

	for (int i = 1; i <= nargs; i++) {
		if (i != 1) { message += " "; }
		message += luaL_tolstring(L, i, nullptr);
	}

	Debug::Log(message.c_str());

	return 0;
}

inline void _registerGlobals(lua_State* L) {
	luaL_Reg globals[] = {
		{ "print", _print },
		{ nullptr, nullptr }
	};

	lua_getglobal(L, "_G");
	luaL_setfuncs(L, globals, 0);
	lua_pop(L, 1);
}

template <class T>
inline T* _userdataPtr(lua_State* L, int index/*, const char* metatable*/) {
	//T** p = (T**)luaL_checkudata(L, 1, Lua::metatableName<T>());
	T** p = (T**)lua_touserdata(L, 1);
	if (p == nullptr) { return nullptr; }

	return *p;
}

template <class T>
inline T* _userdataSharedPtr(lua_State* L, int index/*, const char* metatable*/) {
	//T* p = (T*)luaL_checkudata(L, index, metatable);
	T* p = (T*)lua_touserdata(L, index);
	return p;
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

#pragma endregion

#pragma region con/destructor

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
inline int fromShared(lua_State* L, T ptr) {
	return copyUserdata(L, ptr);
}

template <class T>
inline int deleteSharedPtr(lua_State* L) {
	T* ptr = callerSharedPtr<T>(L, 0);
	if (ptr != nullptr) { ptr->reset(); }
	return 0;
}

template <class T>
inline int newObject(lua_State* L) {
	T** memory = (T**)lua_newuserdata(L, sizeof(T*));
	*memory = new T;

	luaL_getmetatable(L, TypeID<T>::string());
	lua_setmetatable(L, -2);
	return 1;
}

template <class T>
inline int newInterface(lua_State* L) {
	T** memory = (T**)lua_newuserdata(L, sizeof(T*));
	*memory = new T;

	luaL_getmetatable(L, TypeID<T::Interface>::string());
	lua_setmetatable(L, -2);
	return 1;
}

template <class T>
inline int deletePtr(lua_State* L) {
	delete callerPtr<T>(L, 0);
	return 0;
}

template <class T>
inline int reference(lua_State* L) {
	T** memory = (T**)lua_newuserdata(L, sizeof(T*));
	*memory = T::instance();

	luaL_getmetatable(L, TypeID<T>::string());
	lua_setmetatable(L, -2);

	return 1;
}

#pragma endregion

#pragma region meta system

template<typename T> struct _is_shared_ptr : std::false_type {};
template<typename T> struct _is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

template <class T>
inline typename std::enable_if<!_is_shared_ptr<T>::value && !std::is_pointer<T>::value, const char*>::type
metatableName(const T&) {
	return TypeID<T>::string();
}

// get real metatable by virtual function.
template <class T>
inline typename std::enable_if<_is_shared_ptr<T>::value || std::is_pointer<T>::value, const char*>::type
metatableName(const T& o) {
	return o->metatableName();
}

template <class T>
inline void createMetatable(lua_State* L) {
	luaL_newmetatable(L, TypeID<T>::string());

	// duplicate metatable.
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	lua_pop(L, 1);
}

template <class T>
inline void initMetatable(lua_State* L, luaL_Reg* lib, const char* baseClass) {
	luaL_getmetatable(L, TypeID<T>::string());
	luaL_setfuncs(L, lib, 0);

	if (baseClass != nullptr) {
		luaL_getmetatable(L, baseClass);
		lua_setmetatable(L, -2);
	}

	lua_pop(L, 1);
}

#pragma endregion

template <class T>
inline T* callerSharedPtr(lua_State* L, int nargs) {
	if (lua_gettop(L) != nargs + 1) {
		Debug::LogError("invalid function call");
		return nullptr;
	}

	return _userdataSharedPtr<T>(L, 1);
}

template <class T>
inline T* callerPtr(lua_State* L, int nargs) {
	if (lua_gettop(L) != nargs + 1) {
		Debug::LogError("invalid function call");
		return nullptr;
	}

	return _userdataPtr<T>(L, 1);
}

template <class T>
inline int copyUserdata(lua_State* L, const T& value) {
	new(lua_newuserdata(L, sizeof(T))) T(value);

	luaL_getmetatable(L, metatableName(value));
	lua_setmetatable(L, -2);
	return 1;
}

#pragma region push

inline int push(lua_State* L) {
	return 0;
}

template <class T>
inline typename std::enable_if<std::is_enum<T>::value, int>::type
push(lua_State* L, const T& value) {
	return push<int>(L, (int)value);
}

template <class T>
inline typename std::enable_if<!std::is_enum<T>::value, int>::type
push(lua_State* L, const T& value) {
	return copyUserdata<T>(L, value);
}

template <>
inline int push(lua_State* L, const bool& value) {
	lua_pushboolean(L, value);
	return 1;
}

template <>
inline int push(lua_State* L, const int& value) {
	lua_pushinteger(L, value);
	return 1;
}

template <>
inline int push(lua_State* L, const unsigned& value) {
	lua_pushinteger(L, value);
	return 1;
}

template <>
inline int push(lua_State* L, const long& value) {
	lua_pushinteger(L, value);
	return 1;
}

template <>
inline int push(lua_State* L, const unsigned long& value) {
	lua_pushinteger(L, value);
	return 1;
}

template <>
inline int push(lua_State* L, const long long& value) {
	lua_pushinteger(L, value);
	return 1;
}

template <>
inline int push(lua_State* L, const unsigned long long& value) {
	lua_pushinteger(L, value);
	return 1;
}

template <>
inline int push(lua_State* L, const float& value) {
	lua_pushnumber(L, value);
	return 1;
}

template <>
inline int push(lua_State* L, const double& value) {
	lua_pushnumber(L, value);
	return 1;
}

template <>
inline int push(lua_State* L, const std::string& value) {
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

static int pushArray(lua_State* L, const float* ptr, int count) {
	lua_newtable(L);
	for (int i = 0; i < count; ++i) {
		push(L, *ptr++);
		lua_rawseti(L, -2, i + 1);
	}

	return 1;
}

template <>
inline int push(lua_State* L, const Color& value) {
	return pushArray(L, (const float*)&value, 4);
}

template <>
inline int push<glm::vec2>(lua_State* L, const glm::vec2& value) {
	return pushArray(L, (const float*)&value, 2);
}

template <>
inline int push<glm::vec3>(lua_State* L, const glm::vec3& value) {
	return pushArray(L, (const float*)&value, 3);
}

template <>
inline int push<glm::vec4>(lua_State* L, const glm::vec4& value) {
	return pushArray(L, (const float*)&value, 4);
}

template <>
inline int push<glm::quat>(lua_State* L, const glm::quat& value) {
	return pushArray(L, (const float*)&value, 4);
}

template <>
inline int push<glm::mat2>(lua_State* L, const glm::mat2& value) {
	return pushArray(L, (const float*)&value, 4);
}

template <>
inline int push<glm::mat3>(lua_State* L, const glm::mat3& value) {
	return pushArray(L, (const float*)&value, 9);
}

template <>
inline int push<glm::mat4>(lua_State* L, const glm::mat4& value) {
	return pushArray(L, (const float*)&value, 16);
}

template <class T, class... R>
inline int push(lua_State* L, T arg, R... args) {
	int n = push(L, arg);
	n += push(L, args...);
	return n;
}

#pragma endregion

#pragma region get

template <class T>
inline typename std::enable_if<!std::is_enum<T>::value, T>::type
get(lua_State* L, int index) {
	return *_userdataSharedPtr<T>(L, -1);
}

template <>
inline std::string get<std::string>(lua_State* L, int index) {
	luaL_checktype(L, index, LUA_TSTRING);
	return lua_tostring(L, index);
}

template <>
inline bool get<bool>(lua_State* L, int index) {
	luaL_checktype(L, index, LUA_TBOOLEAN);
	return !!lua_toboolean(L, index);
}

template <>
inline int get<int>(lua_State* L, int index) {
	luaL_checktype(L, index, LUA_TNUMBER);
	return (int)lua_tointeger(L, index);
}

template <>
inline unsigned get<unsigned>(lua_State* L, int index) {
	luaL_checktype(L, index, LUA_TNUMBER);
	return (unsigned)lua_tointeger(L, index);
}

template <>
inline long get<long>(lua_State* L, int index) {
	luaL_checktype(L, index, LUA_TNUMBER);
	return (long)lua_tointeger(L, index);
}

template <>
inline unsigned long get<unsigned long>(lua_State* L, int index) {
	luaL_checktype(L, index, LUA_TNUMBER);
	return (unsigned long)lua_tointeger(L, index);
}

template <>
inline long long get<long long>(lua_State* L, int index) {
	luaL_checktype(L, index, LUA_TNUMBER);
	return (long long)lua_tointeger(L, index);
}

template <>
inline unsigned long long get<unsigned long long>(lua_State* L, int index) {
	luaL_checktype(L, index, LUA_TNUMBER);
	return (unsigned long long)lua_tointeger(L, index);
}

template <>
inline float get<float>(lua_State* L, int index) {
	luaL_checktype(L, index, LUA_TNUMBER);
	return (float)lua_tonumber(L, index);
}

template <>
inline double get<double>(lua_State* L, int index) {
	luaL_checktype(L, index, LUA_TNUMBER);
	return lua_tonumber(L, index);
}

template <typename T>
inline typename std::enable_if<std::is_enum<T>::value, T>::type
get(lua_State* L, int index) {
	return (T)get<int>(L, index);
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

template <>
inline glm::vec2 get<glm::vec2>(lua_State* L, int index) {
	return _glmConvert<glm::vec2>(L, index);
}

template <>
inline glm::vec3 get<glm::vec3>(lua_State* L, int index) {
	return _glmConvert<glm::vec3>(L, index);
}

template <>
inline glm::vec4 get<glm::vec4>(lua_State* L, int index) {
	return _glmConvert<glm::vec4>(L, index);
}

template <>
inline glm::quat get<glm::quat>(lua_State* L, int index) {
	return _glmConvert<glm::quat>(L, index);
}

template <>
inline glm::mat2 get<glm::mat2>(lua_State* L, int index) {
	return _glmConvert<glm::mat2>(L, index);
}

template <>
inline glm::mat3 get<glm::mat3>(lua_State* L, int index) {
	return _glmConvert<glm::mat3>(L, index);
}

template <>
inline glm::mat4 get<glm::mat4>(lua_State* L, int index) {
	return _glmConvert<glm::mat4>(L, index);
}

#pragma endregion

#pragma region function

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

template <class R, class... Args>
class IFunc : public _FBase <R, Args...> {
public:
	IFunc(lua_State* L) : _FBase(L) {}
};

template <class... Args>
class IFunc<void, Args...> : public _FBase<void, Args...> {
public:
	IFunc(lua_State* L) : _FBase(L) {}

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
using Func = std::shared_ptr<IFunc<R, Args...>>;

template <class R, class... Args>
Func<R, Args...> make_func(lua_State* L) {
	if (lua_type(L, -1) != LUA_TFUNCTION) {
		Debug::LogError("top of stack must be a function");
		return nullptr;
	}

	return std::make_shared<Func<R, Args...>::element_type>(L);
}

#pragma endregion

}	// namespace Lua
