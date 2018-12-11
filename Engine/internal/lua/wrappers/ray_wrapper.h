// Warning: this file is generated by wrapper.py.

#pragma once

#include "ray.h"

#include "lua++.h"
#include "tools/string.h"

class Ray_Wrapper {
	static int NewRay(lua_State* L) {
		if (Lua::checkArguments(L, 1)) {
			return Lua::newObject<Ray>(L);
		}

		if (Lua::checkArguments<glm::vec3, glm::vec3>(L, 1)) {
			glm::vec3 direction = Lua::get<glm::vec3>(L, 2);
			glm::vec3 origin = Lua::get<glm::vec3>(L, 1);
		
			return Lua::newObject<Ray>(L, origin, direction);
		}

		Debug::LogError("failed to call \"Ray\", invalid arguments.");
		return 0;
	}

	static int ToString(lua_State* L) {
		Ray* _p = Lua::callerPtr<Ray>(L);

		lua_pushstring(L, String::Format("Ray@0x%p", _p).c_str());
		return 1;
	}

	// glm::vec3 GetPoint(float distance)
	static int GetPoint(lua_State* L) {
		Ray* _p = Lua::callerPtr<Ray>(L);
		float distance = Lua::get<float>(L, 2);
		
		return Lua::push(L, _p->GetPoint(distance));
	}

	// void SetOrigin(const glm::vec3& value)
	static int SetOrigin(lua_State* L) {
		Ray* _p = Lua::callerPtr<Ray>(L);
		glm::vec3 value = Lua::get<glm::vec3>(L, 2);
		
		_p->SetOrigin(value);
		return 0;
	}

	// void SetDirection(const glm::vec3& value) { direction_ = glm::normalize(value)
	static int SetDirection(lua_State* L) {
		Ray* _p = Lua::callerPtr<Ray>(L);
		glm::vec3 value = Lua::get<glm::vec3>(L, 2);
		
		_p->SetDirection(value);
		return 0;
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<Ray>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		funcs.push_back(luaL_Reg { "NewRay", NewRay });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deletePtr<Ray> },
			{ "__tostring", ToString }, 
			{ "GetPoint", GetPoint },
			{ "SetOrigin", SetOrigin },
			{ "SetDirection", SetDirection },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<Ray>(L, metalib, nullptr);
	}
};
