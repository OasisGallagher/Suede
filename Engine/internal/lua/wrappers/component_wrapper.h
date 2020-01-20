// Warning: this file is generated by wrapper.py.

#pragma once

#include "component.h"

#include "lua++.h"
#include "tools/string.h"

class Component_Wrapper {
	static int ToString(lua_State* L) {
		Component* _p = Lua::callerPtr<Component>(L);

		lua_pushstring(L, String::Format("Component@0x%p", _p).c_str());
		return 1;
	}

	static int ToStringStatic(lua_State* L) {
		lua_pushstring(L, "static Component");
		return 1;
	}

	static int ComponentStatic(lua_State* L) {
		lua_newtable(L);

		luaL_Reg funcs[] = {
			{ "GetComponentGUID", GetComponentGUID },
			{ "ClassNameToGUID", ClassNameToGUID },
			{"__tostring", ToStringStatic },
			{ nullptr, nullptr }
		};

		luaL_setfuncs(L, funcs, 0);

		return 1;
	}
	// virtual void Awake()
	static int Awake(lua_State* L) {
		Component* _p = Lua::callerPtr<Component>(L);
		_p->Awake();
		return 0;
	}

	// virtual void Update()
	static int Update(lua_State* L) {
		Component* _p = Lua::callerPtr<Component>(L);
		_p->Update();
		return 0;
	}

	// bool GetEnabled()
	static int GetEnabled(lua_State* L) {
		Component* _p = Lua::callerPtr<Component>(L);
		return Lua::push(L, _p->GetEnabled());
	}

	// void SetEnabled(bool value)
	static int SetEnabled(lua_State* L) {
		Component* _p = Lua::callerPtr<Component>(L);
		bool value = Lua::get<bool>(L, 2);
		
		_p->SetEnabled(value);
		return 0;
	}

	// void CullingUpdate()
	static int CullingUpdate(lua_State* L) {
		Component* _p = Lua::callerPtr<Component>(L);
		_p->CullingUpdate();
		return 0;
	}

	// int GetUpdateStrategy()
	static int GetUpdateStrategy(lua_State* L) {
		Component* _p = Lua::callerPtr<Component>(L);
		return Lua::push(L, _p->GetUpdateStrategy());
	}

	// static suede_guid GetComponentGUID()
	static int GetComponentGUID(lua_State* L) {
		return Lua::push(L, Component::GetComponentGUID());
	}

	// static suede_guid ClassNameToGUID(const char* className)
	static int ClassNameToGUID(lua_State* L) {
		std::string className = Lua::get<std::string>(L, 1);
		
		return Lua::push(L, Component::ClassNameToGUID(className.c_str()));
	}

	// virtual bool AllowMultiple()
	static int AllowMultiple(lua_State* L) {
		Component* _p = Lua::callerPtr<Component>(L);
		return Lua::push(L, _p->AllowMultiple());
	}

	// virtual bool IsComponentType(suede_guid guid) const { return guid == GetComponentGUID()
	// virtual bool IsComponentType(const char* name) const { return strcmp(name, GetComponentName())
	static int IsComponentType(lua_State* L) {
		Component* _p = Lua::callerPtr<Component>(L);
		if (Lua::checkArguments<suede_guid>(L, 2)) {
			suede_guid guid = Lua::get<suede_guid>(L, 2);
			
			return Lua::push(L, _p->IsComponentType(guid));
		}

		if (Lua::checkArguments<std::string>(L, 2)) {
			std::string name = Lua::get<std::string>(L, 2);
			
			return Lua::push(L, _p->IsComponentType(name.c_str()));
		}

		Debug::LogError("failed to call \"IsComponentType\", invalid arguments.");
		return 0;
	}

	// virtual suede_guid GetComponentInstanceGUID() const { return GetComponentGUID()
	static int GetComponentInstanceGUID(lua_State* L) {
		Component* _p = Lua::callerPtr<Component>(L);
		return Lua::push(L, _p->GetComponentInstanceGUID());
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<Component>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		fields.push_back(luaL_Reg{ "Component", ComponentStatic });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deletePtr<Component> },
			{ "__tostring", ToString }, 
			{ "Awake", Awake },
			{ "Update", Update },
			{ "GetEnabled", GetEnabled },
			{ "SetEnabled", SetEnabled },
			{ "CullingUpdate", CullingUpdate },
			{ "GetUpdateStrategy", GetUpdateStrategy },
			{ "AllowMultiple", AllowMultiple },
			{ "IsComponentType", IsComponentType },
			{ "GetComponentInstanceGUID", GetComponentInstanceGUID },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<Component>(L, metalib, TypeID<Object>::string());
	}
};