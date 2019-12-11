// Warning: this file is generated by wrapper.py.

#pragma once

#include "light.h"

#include "lua++.h"
#include "tools/string.h"

class Light_Wrapper {
	static int NewLight(lua_State* L) {
		return Lua::newObject<Light>(L);
	}

	static int ToString(lua_State* L) {
		Light* _p = Lua::callerPtr<Light>(L);

		lua_pushstring(L, String::Format("Light@0x%p", _p).c_str());
		return 1;
	}

	// void SetType(LightType value)
	static int SetType(lua_State* L) {
		Light* _p = Lua::callerPtr<Light>(L);
		LightType value = Lua::get<LightType>(L, 2);
		
		_p->SetType(value);
		return 0;
	}

	// LightType GetType()
	static int GetType(lua_State* L) {
		Light* _p = Lua::callerPtr<Light>(L);
		return Lua::push(L, _p->GetType());
	}

	// void SetImportance(LightImportance value)
	static int SetImportance(lua_State* L) {
		Light* _p = Lua::callerPtr<Light>(L);
		LightImportance value = Lua::get<LightImportance>(L, 2);
		
		_p->SetImportance(value);
		return 0;
	}

	// LightImportance GetImportance()
	static int GetImportance(lua_State* L) {
		Light* _p = Lua::callerPtr<Light>(L);
		return Lua::push(L, _p->GetImportance());
	}

	// void SetColor(const Color& value)
	static int SetColor(lua_State* L) {
		Light* _p = Lua::callerPtr<Light>(L);
		Color value = Lua::get<Color>(L, 2);
		
		_p->SetColor(value);
		return 0;
	}

	// Color GetColor()
	static int GetColor(lua_State* L) {
		Light* _p = Lua::callerPtr<Light>(L);
		return Lua::push(L, _p->GetColor());
	}

	// void SetIntensity(float value)
	static int SetIntensity(lua_State* L) {
		Light* _p = Lua::callerPtr<Light>(L);
		float value = Lua::get<float>(L, 2);
		
		_p->SetIntensity(value);
		return 0;
	}

	// float GetIntensity()
	static int GetIntensity(lua_State* L) {
		Light* _p = Lua::callerPtr<Light>(L);
		return Lua::push(L, _p->GetIntensity());
	}

	// int GetUpdateStrategy()
	static int GetUpdateStrategy(lua_State* L) {
		Light* _p = Lua::callerPtr<Light>(L);
		return Lua::push(L, _p->GetUpdateStrategy());
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<Light>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		funcs.push_back(luaL_Reg { "NewLight", NewLight });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deletePtr<Light> },
			{ "__tostring", ToString }, 
			{ "SetType", SetType },
			{ "GetType", GetType },
			{ "SetImportance", SetImportance },
			{ "GetImportance", GetImportance },
			{ "SetColor", SetColor },
			{ "GetColor", GetColor },
			{ "SetIntensity", SetIntensity },
			{ "GetIntensity", GetIntensity },
			{ "GetUpdateStrategy", GetUpdateStrategy },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<Light>(L, metalib, TypeID<Component>::string());
	}
};
