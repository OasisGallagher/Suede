// Warning: this file is generated by wrapper.py.

#pragma once

#include "gameobject.h"

#include "lua++.h"
#include "tools/string.h"

class GameObject_Wrapper {
	static int NewGameObject(lua_State* L) {
		return Lua::fromShared(L, ::NewGameObject());
	}

	static int ToString(lua_State* L) {
		GameObject& _p = *Lua::callerSharedPtr<GameObject>(L, 0);
		lua_pushstring(L, String::Format("GameObject@0x%p", _p.get()).c_str());
		return 1;
	}

	// bool GetActive()
	static int GetActive(lua_State* L) {
		GameObject& _p = *Lua::callerSharedPtr<GameObject>(L, 0);
		return Lua::push(L, _p->GetActive());
	}

	// void SetActiveSelf(bool value)
	static int SetActiveSelf(lua_State* L) {
		GameObject& _p = *Lua::callerSharedPtr<GameObject>(L, 1);
		bool value = Lua::get<bool>(L, 2);
		_p->SetActiveSelf(value);
		return 0;
	}

	// bool GetActiveSelf()
	static int GetActiveSelf(lua_State* L) {
		GameObject& _p = *Lua::callerSharedPtr<GameObject>(L, 0);
		return Lua::push(L, _p->GetActiveSelf());
	}

	// int GetUpdateStrategy()
	static int GetUpdateStrategy(lua_State* L) {
		GameObject& _p = *Lua::callerSharedPtr<GameObject>(L, 0);
		return Lua::push(L, _p->GetUpdateStrategy());
	}

	// bool SetTag(const std::string& value)
	static int SetTag(lua_State* L) {
		GameObject& _p = *Lua::callerSharedPtr<GameObject>(L, 1);
		std::string value = Lua::get<std::string>(L, 2);
		return Lua::push(L, _p->SetTag(value));
	}

	// std::string GetName()
	static int GetName(lua_State* L) {
		GameObject& _p = *Lua::callerSharedPtr<GameObject>(L, 0);
		return Lua::push(L, _p->GetName());
	}

	// void SetName(const std::string& value)
	static int SetName(lua_State* L) {
		GameObject& _p = *Lua::callerSharedPtr<GameObject>(L, 1);
		std::string value = Lua::get<std::string>(L, 2);
		_p->SetName(value);
		return 0;
	}

	// void CullingUpdate()
	static int CullingUpdate(lua_State* L) {
		GameObject& _p = *Lua::callerSharedPtr<GameObject>(L, 0);
		_p->CullingUpdate();
		return 0;
	}

	// void RenderingUpdate()
	static int RenderingUpdate(lua_State* L) {
		GameObject& _p = *Lua::callerSharedPtr<GameObject>(L, 0);
		_p->RenderingUpdate();
		return 0;
	}

	// Transform GetTransform()
	static int GetTransform(lua_State* L) {
		GameObject& _p = *Lua::callerSharedPtr<GameObject>(L, 0);
		return Lua::push(L, _p->GetTransform());
	}

	// void RecalculateBounds(int flags = RecalculateBoundsFlagsAll)
	static int RecalculateBounds(lua_State* L) {
		GameObject& _p = *Lua::callerSharedPtr<GameObject>(L, 1);
		int flags = Lua::get<int>(L, 2);
		_p->RecalculateBounds(flags);
		return 0;
	}

	// void RecalculateUpdateStrategy()
	static int RecalculateUpdateStrategy(lua_State* L) {
		GameObject& _p = *Lua::callerSharedPtr<GameObject>(L, 0);
		_p->RecalculateUpdateStrategy();
		return 0;
	}

	// Component AddComponent(const char* name)
	static int AddComponent(lua_State* L) {
		GameObject& _p = *Lua::callerSharedPtr<GameObject>(L, 1);
		std::string name = Lua::get<std::string>(L, 2);
		return Lua::push(L, _p->AddComponent(name.c_str()));
	}

	// Component GetComponent(suede_guid guid)
	static int GetComponent(lua_State* L) {
		GameObject& _p = *Lua::callerSharedPtr<GameObject>(L, 1);
		suede_guid guid = Lua::get<suede_guid>(L, 2);
		return Lua::push(L, _p->GetComponent(guid));
	}

	// Component GetComponent(const char* name)
	static int GetComponent2(lua_State* L) {
		GameObject& _p = *Lua::callerSharedPtr<GameObject>(L, 1);
		std::string name = Lua::get<std::string>(L, 2);
		return Lua::push(L, _p->GetComponent(name.c_str()));
	}

	// std::vector<Component> GetComponents(suede_guid guid)
	static int GetComponents(lua_State* L) {
		GameObject& _p = *Lua::callerSharedPtr<GameObject>(L, 1);
		suede_guid guid = Lua::get<suede_guid>(L, 2);
		return Lua::pushList(L, _p->GetComponents(guid));
	}

	// std::vector<Component> GetComponents(const char* name)
	static int GetComponents2(lua_State* L) {
		GameObject& _p = *Lua::callerSharedPtr<GameObject>(L, 1);
		std::string name = Lua::get<std::string>(L, 2);
		return Lua::pushList(L, _p->GetComponents(name.c_str()));
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<GameObject>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		funcs.push_back(luaL_Reg { "NewGameObject", NewGameObject });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deleteSharedPtr<GameObject> },
			{ "__tostring", ToString }, 
			{ "GetActive", GetActive },
			{ "SetActiveSelf", SetActiveSelf },
			{ "GetActiveSelf", GetActiveSelf },
			{ "GetUpdateStrategy", GetUpdateStrategy },
			{ "SetTag", SetTag },
			{ "GetName", GetName },
			{ "SetName", SetName },
			{ "CullingUpdate", CullingUpdate },
			{ "RenderingUpdate", RenderingUpdate },
			{ "GetTransform", GetTransform },
			{ "RecalculateBounds", RecalculateBounds },
			{ "RecalculateUpdateStrategy", RecalculateUpdateStrategy },
			{ "AddComponent", AddComponent },
			{ "GetComponent", GetComponent },
			{ "GetComponent2", GetComponent2 },
			{ "GetComponents", GetComponents },
			{ "GetComponents2", GetComponents2 },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<GameObject>(L, metalib, TypeID<Object>::string());
	}
};
