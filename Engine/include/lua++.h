#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cassert>
#include <type_traits>

#include "tools/enum.h"
#include "math/mathf.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "math/vector4.h"
#include "math/matrix4.h"
#include "math/quaternion.h"

#include "debug/debug.h"
#include "tools/typeid.h"

#include "defines.h"

extern "C" {
#include "lua/lua.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"
}

namespace Lua {

#pragma region internal helpers

class _StackChecker {
	int top;
	lua_State* L;

public:
	_StackChecker(lua_State* L) : L(L) {
		top = lua_gettop(L);
	}

	~_StackChecker() {
		int n = lua_gettop(L);
		assert(top == n && "lua stack unblanced");
	}
};

#ifdef _DEBUG
#define _CHECK_LUA_STACK(L)	_StackChecker _checker(L);
#else
#define _CHECK_LUA_STACK(L)
#endif

// get message from stack.
inline std::string _message(lua_State* L) {
	std::string message;
	int nargs = lua_gettop(L);

	for (int i = 1; i <= nargs; i++) {
		if (i != 1) { message += " "; }
		message += luaL_tolstring(L, i, nullptr);
	}

	return message;
}


inline int _log(lua_State *L) {
	Debug::Log(_message(L).c_str());
	return 0;
}

inline int _warning(lua_State *L) {
	Debug::LogWarning(_message(L).c_str());
	return 0;
}

inline int _error(lua_State *L) {
	Debug::LogError(_message(L).c_str());
	return 0;
}

inline void _registerGlobals(lua_State* L) {
	luaL_Reg globals[] = {
		{ "print", _log },
		{ "warning", _warning },
		{ "error", _error },
		{ nullptr, nullptr }
	};

	lua_getglobal(L, "_G");
	luaL_setfuncs(L, globals, 0);
	lua_pop(L, 1);
}

// get caller as raw pointer.
template <class T>
inline T* _userdataPtr(lua_State* L, int index, const char* metatable) {
#ifdef _DEBUG
	if (!checkMetatable(L, index, metatable)) {
		return nullptr;
	}
#endif

	T** p = (T**)lua_touserdata(L, index);
	if (p == nullptr) { return nullptr; }

	return *p;
}

// get caller as ref_ptr.
template <class T>
inline T* _userdataRefPtr(lua_State* L, int index, const char* metatable) {
#ifdef _DEBUG
	if (!checkMetatable(L, index, metatable)) {
		return nullptr;
	}
#endif

	T* p = (T*)lua_touserdata(L, index);
	return p;
}

template <class T>
static T _mathConvert(lua_State* L, int index) {
	T ans;
	float* ptr = (float*)&ans;
	std::vector<float> values = getList<float>(L, index);
	for (int i = 0; i < Mathf::Min(sizeof(T) / sizeof(float), values.size()); ++i) {
		*ptr++ = values[i];
	}

	return ans;
}

#pragma endregion

#pragma region con/destructors

static void initialize(lua_State* L, luaL_Reg* libs, const char* entry) {
	_CHECK_LUA_STACK(L);

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

	lua_atpanic(L, _error);

	_registerGlobals(L);
	
	if (luaL_dofile(L, entry) != 0) {
		_error(L);
		lua_pop(L, 1);
	}
}

// construct userdata from ref_ptr.
template <class T>
inline int fromRef(lua_State* L, T ptr) {
	return copyUserdata(L, ptr);
}

// unref caller ref_ptr.
template <class T>
inline int deleteRefPtr(lua_State* L) {
	T* ptr = callerRefPtr<T>(L, 0);
	if (ptr != nullptr) { ptr->reset(); }
	return 0;
}

// construct userdata from raw pointer.
template <class T, class... R>
inline int newObject(lua_State* L, R... args) {
	T** memory = (T**)lua_newuserdata(L, sizeof(T*));
	*memory = new T(args...);

	luaL_getmetatable(L, TypeID<T>::string());
	lua_setmetatable(L, -2);
	return 1;
}

// delete caller raw pointer.
template <class T>
inline int deletePtr(lua_State* L) {
	delete callerPtr<T>(L, 0);
	return 0;
}

// construct userdata by sharing C++ side pointer.
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

// check non-enum and non-vector argument type.
template <class T>
inline typename std::enable_if<!std::is_enum<T>::value && !suede_is_vector<T>::value, bool>::type
_checkArgumentType(lua_State* L, int index) {
	return checkMetatable(L, index, TypeID<T>::string());
}

inline bool _checkArgumentType(lua_State* L, int index) {
	return true;
}

template <>
inline bool _checkArgumentType<std::string>(lua_State* L, int index) {
	return lua_type(L, index) == LUA_TSTRING;
}

template <>
inline bool _checkArgumentType<bool>(lua_State* L, int index) {
	return lua_type(L, index) == LUA_TBOOLEAN;
}

template <>
inline bool _checkArgumentType<int>(lua_State* L, int index) {
	return lua_type(L, index) == LUA_TNUMBER;
}

template <>
inline bool _checkArgumentType<unsigned>(lua_State* L, int index) {
	return lua_type(L, index) == LUA_TNUMBER;
}

template <>
inline bool _checkArgumentType<long>(lua_State* L, int index) {
	return lua_type(L, index) == LUA_TNUMBER;
}

template <>
inline bool _checkArgumentType<unsigned long>(lua_State* L, int index) {
	return lua_type(L, index) == LUA_TNUMBER;
}

template <>
inline bool _checkArgumentType<long long>(lua_State* L, int index) {
	return lua_type(L, index) == LUA_TNUMBER;
}

template <>
inline bool _checkArgumentType<unsigned long long>(lua_State* L, int index) {
	return lua_type(L, index) == LUA_TNUMBER;
}

template <>
inline bool _checkArgumentType<float>(lua_State* L, int index) {
	return lua_type(L, index) == LUA_TNUMBER;
}

// check enum argument type.
template <class T>
inline typename std::enable_if<std::is_enum<T>::value, bool>::type
_checkArgumentType(lua_State* L, int index) {
	return lua_type(L, index) == LUA_TNUMBER;
}

template <class T>
static bool _checkListElementType(lua_State* L, int index, int count = -1) {
	if (lua_type(L, index) != LUA_TTABLE) {
		return false;
	}

	bool valid = true;
	int size = (int)lua_rawlen(L, index);

	int i = 1;
	for (; valid && i <= size; ++i) {
		lua_geti(L, -1, i);
		
		bool nil = lua_isnil(L, -1);

		if (nil && !suede_is_ptr<T>::value) { valid = false; }
		if (!nil && !_checkArgumentType<T>(L, -1)) { valid = false; }

		lua_pop(L, 1);
	}

	return valid && (count == -1 || count == (i - 1));
}

template <>
inline bool _checkArgumentType<Vector2>(lua_State* L, int index) {
	return _checkListElementType<float>(L, index, 2);
}

template <>
inline bool _checkArgumentType<Vector3>(lua_State* L, int index) {
	return _checkListElementType<float>(L, index, 3);
}

template <>
inline bool _checkArgumentType<Vector4>(lua_State* L, int index) {
	return _checkListElementType<float>(L, index, 4);
}

template <>
inline bool _checkArgumentType<Quaternion>(lua_State* L, int index) {
	return _checkListElementType<float>(L, index, 4);
}

template <>
inline bool _checkArgumentType<Matrix4>(lua_State* L, int index) {
	return _checkListElementType<float>(L, index, 16);
}

// check list argument type.
template <class T>
inline typename std::enable_if<suede_is_vector<T>::value, bool>::type
_checkArgumentType(lua_State* L, int index) {
	return _checkListElementType<typename T::value_type>(L, index);
}

inline bool _checkArgumentsTypes(lua_State* L, int index) {
	return true;
}

template <class T>
inline bool _checkArgumentsTypes(lua_State* L, int index) {
	return _checkArgumentType<T>(L, index);
}

template <class T, class U, class... R>
inline bool _checkArgumentsTypes(lua_State* L, int index) {
	return _checkArgumentsTypes<T>(L, index) && _checkArgumentsTypes<U, R...>(L, index + 1);
}

// check arguments count and types.
inline bool checkArguments(lua_State* L, int index) {
	return (lua_gettop(L) == index - 1);
}

// check arguments count and types.
template <class T>
inline bool checkArguments(lua_State* L, int index) {
	return (lua_gettop(L) == index) && _checkArgumentsTypes<T>(L, index);
}

// check arguments count and types.
template <class T, class U, class... R>
inline bool checkArguments(lua_State* L, int index) {
	return (lua_gettop(L) == sizeof...(R) + 2 + (index - 1)) && _checkArgumentsTypes<T, U, R...>(L, index);
}

// check whether `metatable` is userdata's metatable or base.
inline bool checkMetatable(lua_State* L, int index, const char* metatable) {
	luaL_getmetatable(L, metatable);
	if (index < 0) { --index; }
	int n = 1, status = 0;

	for (; ; ) {
		if (!lua_getmetatable(L, index)) { break; }
		++n;

		if (lua_rawequal(L, -1, -n)) { status = 1; break; }

		// check base "class" metatable.
		index = -1;
	}

	lua_pop(L, n);

	return !!status;
}

// get non-pointer metatable name.
template <class T>
inline typename std::enable_if<!suede_is_ptr<T>::value, const char*>::type
metatableName(const T&) {
	return TypeID<T>::string();
}

// get real metatable by virtual function.
template <class T>
inline typename std::enable_if<suede_is_ptr<T>::value, const char*>::type
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

// get caller as ref_ptr.
template <class T>
inline T* callerRefPtr(lua_State* L, const char* metatable = nullptr) {
	if (metatable == nullptr) {
		metatable = TypeID<T>::string();
	}

	return _userdataRefPtr<T>(L, 1, metatable);
}

// get caller as raw pointer.
template <class T>
inline T* callerPtr(lua_State* L, const char* metatable = nullptr) {
	if (metatable == nullptr) {
		metatable = TypeID<T>::string();
	}

	return _userdataPtr<T>(L, 1, metatable);
}

// copy construct userdata.
/*
template <class T>
inline int copyUserdata(lua_State* L, const T& value) {
	new(lua_newuserdata(L, sizeof(T))) T(value);

	luaL_getmetatable(L, metatableName(value));
	lua_setmetatable(L, -2);
	return 1;
}*/

template <class T>
inline typename std::enable_if<suede_is_ptr<T>::value, int>::type
copyUserdata(lua_State* L, const T& value) {
	if (!value) { lua_pushnil(L); }
	else {
		new(lua_newuserdata(L, sizeof(T))) T(value);
		luaL_getmetatable(L, metatableName(value));
		lua_setmetatable(L, -2);
	}

	return 1;
}

template <class T>
inline typename std::enable_if<!suede_is_ptr<T>::value, int>::type
copyUserdata(lua_State* L, const T& value) {
	new(lua_newuserdata(L, sizeof(T))) T(value);

	luaL_getmetatable(L, metatableName(value));
	lua_setmetatable(L, -2);
	return 1;
}

#pragma region pushers

inline int push(lua_State* L) {
	return 0;
}

// push enum value.
template <class T>
inline typename std::enable_if<std::is_enum<T>::value, int>::type
push(lua_State* L, const T& value) {
	return push<int>(L, (int)value);
}

// push non-enum userdata value.
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
inline int push<Vector2>(lua_State* L, const Vector2& value) {
	return pushArray(L, (const float*)&value, 2);
}

template <>
inline int push<Vector3>(lua_State* L, const Vector3& value) {
	return pushArray(L, (const float*)&value, 3);
}

template <>
inline int push<Vector4>(lua_State* L, const Vector4& value) {
	return pushArray(L, (const float*)&value, 4);
}

template <>
inline int push<Quaternion>(lua_State* L, const Quaternion& value) {
	return pushArray(L, (const float*)&value, 4);
}

template <>
inline int push<Matrix4>(lua_State* L, const Matrix4& value) {
	return pushArray(L, (const float*)&value, 16);
}

template <class T, class... R>
inline int push(lua_State* L, T arg, R... args) {
	int n = push(L, arg);
	n += push(L, args...);
	return n;
}

#pragma endregion

#pragma region getters

// get std::shared_ptr/ref_ptr/raw pointer value.
template <class T>
inline typename std::enable_if<suede_is_ptr<T>::value, T>::type
get(lua_State* L, int index) {
	T* p = _userdataRefPtr<T>(L, index, TypeID<T>::string());
	if (p == nullptr) { return nullptr; }
	return *p;
}

// get object value(non-enum, non-better-enum, non-pointer).
template <class T>
inline typename std::enable_if<!std::is_enum<T>::value && !suede_is_ptr<T>::value && !std::is_base_of<BetterEnumBase, T>::value, T>::type
get(lua_State* L, int index) {
	T* p = _userdataPtr<T>(L, index, TypeID<T>::string());
	if (p == nullptr) {
		Debug::LogError("argument at #%d does not has metatable %s.", index, TypeID<T>::string());
		return T();
	}

	return *p;
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

// get enum value.
template <class T>
inline typename std::enable_if<std::is_enum<T>::value, T>::type
get(lua_State* L, int index) {
	return (T)get<int>(L, index);
}

// get better-enum value.
template <class T>
inline typename std::enable_if<std::is_base_of<BetterEnumBase, T>::value, T>::type
get(lua_State* L, int index) {
	return T::from_int(get<int>(L, index));
}

template <class T>
static std::vector<T> getList(lua_State* L, int index) {
	std::vector<T> container;
	if (lua_type(L, index) != LUA_TTABLE) {
		Debug::LogError("value at #%d is not a table.", index);
		return container;
	}

	int size = (int)lua_rawlen(L, index);

	for (int i = 1; i <= size; ++i) {
		lua_geti(L, -1, i);

		if (lua_isnil(L, -1) && !suede_is_ptr<T>::value) {
			size = i - 1;	// exit loop.
			Debug::LogWarning("list truncated at #%d.", i);
		}

		container.push_back(get<T>(L, -1));
		lua_pop(L, 1);
	}

	return container;
}

template <>
inline Vector2 get<Vector2>(lua_State* L, int index) {
	return _mathConvert<Vector2>(L, index);
}

template <>
inline Vector3 get<Vector3>(lua_State* L, int index) {
	return _mathConvert<Vector3>(L, index);
}

template <>
inline Vector4 get<Vector4>(lua_State* L, int index) {
	return _mathConvert<Vector4>(L, index);
}

template <>
inline Quaternion get<Quaternion>(lua_State* L, int index) {
	return _mathConvert<Quaternion>(L, index);
}

template <>
inline Matrix4 get<Matrix4>(lua_State* L, int index) {
	return _mathConvert<Matrix4>(L, index);
}

#pragma endregion

#pragma region function

template <class... Args>
static bool _invokeCurrentFunction(lua_State* L, Args... args) {
	push(L, args...);
	int r = lua_pcall(L, sizeof...(Args), 0, 0);
	if (r != LUA_OK) {
		Debug::LogError("invoke function failed(%d): %s.", r, lua_tostring(L, -1));
		lua_pop(L, 1);
		return false;
	}

	return true;
}

static int _getGlobalFunction(lua_State* L, const std::string& name) {
	size_t pos = 0, n = 0;
	for (; ; ) {
		size_t current = name.find('.', pos);

		std::string substr = name.substr(pos, current);
		if (pos == 0) {
			lua_getglobal(L, substr.c_str());
		}
		else {
			lua_getfield(L, -1, substr.c_str());
		}

		++n;

		if (current == std::string::npos) {
			break;
		}

		pos = current + 1;
	}

	return (int)n;
}

static int getGlobalFunctionRef(lua_State* L, const std::string& name) {
	assert(!name.empty() && "invalid name");
	_CHECK_LUA_STACK(L);

	int n = _getGlobalFunction(L, name);
	int ref = luaL_ref(L, LUA_REGISTRYINDEX);
	lua_pop(L, n - 1);

	return ref;
}

template <class... Args>
inline void invokeGlobalFunction(lua_State* L, int ref, Args... args) {
	_CHECK_LUA_STACK(L);
	lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
	_invokeCurrentFunction(L, args...);
}

template <class... Args>
inline void invokeGlobalFunction(lua_State* L, const char* name, Args... args) {
	_CHECK_LUA_STACK(L);
	int n = _getGlobalFunction(L, name);
	_invokeCurrentFunction(L, args...);
	lua_pop(L, n - 1);
}

// lua function wrapper base.
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

		R r;
		if (lua_pcall(L, sizeof...(Args), 1, 0) != 0) {
			_error(L);
		}
		else {
			r = Lua::get<R>(L, -1);
			lua_pop(L, 1);
		}

		ref_ = luaL_ref(L, LUA_REGISTRYINDEX);
		return r;
	}

protected:
	int ref_;
	lua_State* L;
};

// lua function wrapper.
template <class R, class... Args>
class IFunc : public _FBase <R, Args...> {
public:
	IFunc(lua_State* L) : _FBase(L) {}
};

// lua function wrapper(return = void).
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

// create lua function wrapper.
template <class R, class... Args>
Func<R, Args...> make_func(lua_State* L) {
	if (lua_type(L, -1) != LUA_TFUNCTION) {
		Debug::LogError("invalid function at #-1.");
		return nullptr;
	}

	return std::make_shared<IFunc<R, Args...>>(L);
}

#pragma endregion

}	// namespace Lua
