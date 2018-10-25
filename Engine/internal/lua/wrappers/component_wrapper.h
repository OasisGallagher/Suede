// Warning: this file is generated by wrapper.py.

#pragma once

#include "lua++.h"
#include "component.h"

class Component_Wrapper {
	// virtual void Awake()
	static int Awake(lua_State* L) {
		Component& _p = *Lua::callerSharedPtr<Component>(L, 0);
		_p->Awake();
		return 0;
	}

	// virtual void OnRenderImage(RenderTexture src, RenderTexture dest, const Rect& normalizedRect)
	static int OnRenderImage(lua_State* L) {
		Component& _p = *Lua::callerSharedPtr<Component>(L, 3);
		Rect normalizedRect = Lua::get<Rect>(L, 4);
		RenderTexture dest = Lua::get<RenderTexture>(L, 3);
		RenderTexture src = Lua::get<RenderTexture>(L, 2);
		_p->OnRenderImage(src, dest, normalizedRect);
		return 0;
	}

	// bool GetEnabled() const
	static int GetEnabled(lua_State* L) {
		Component& _p = *Lua::callerSharedPtr<Component>(L, 0);
		return Lua::push(L, _p->GetEnabled());
	}

	// void SetEnabled(bool value)
	static int SetEnabled(lua_State* L) {
		Component& _p = *Lua::callerSharedPtr<Component>(L, 1);
		bool value = Lua::get<bool>(L, 2);
		_p->SetEnabled(value);
		return 0;
	}

	// void SetGameObject(GameObject value)
	static int SetGameObject(lua_State* L) {
		Component& _p = *Lua::callerSharedPtr<Component>(L, 1);
		GameObject value = Lua::get<GameObject>(L, 2);
		_p->SetGameObject(value);
		return 0;
	}

	// GameObject GetGameObject()
	static int GetGameObject(lua_State* L) {
		Component& _p = *Lua::callerSharedPtr<Component>(L, 0);
		return Lua::push(L, _p->GetGameObject());
	}

	// Transform GetTransform()
	static int GetTransform(lua_State* L) {
		Component& _p = *Lua::callerSharedPtr<Component>(L, 0);
		return Lua::push(L, _p->GetTransform());
	}

	// void CullingUpdate()
	static int CullingUpdate(lua_State* L) {
		Component& _p = *Lua::callerSharedPtr<Component>(L, 0);
		_p->CullingUpdate();
		return 0;
	}

	// void RenderingUpdate()
	static int RenderingUpdate(lua_State* L) {
		Component& _p = *Lua::callerSharedPtr<Component>(L, 0);
		_p->RenderingUpdate();
		return 0;
	}

	// int GetUpdateStrategy()
	static int GetUpdateStrategy(lua_State* L) {
		Component& _p = *Lua::callerSharedPtr<Component>(L, 0);
		return Lua::push(L, _p->GetUpdateStrategy());
	}

	// virtual bool AllowMultiple() const { return false }
	static int AllowMultiple(lua_State* L) {
		Component& _p = *Lua::callerSharedPtr<Component>(L, 0);
		return Lua::push(L, _p->AllowMultiple());
	}

	// virtual bool IsComponentType(suede_guid guid) const { return guid == GetComponentGUID() }
	static int IsComponentType(lua_State* L) {
		Component& _p = *Lua::callerSharedPtr<Component>(L, 1);
		suede_guid guid = Lua::get<suede_guid>(L, 2);
		return Lua::push(L, _p->IsComponentType(guid));
	}

	// virtual bool IsComponentType(const char* name) const { return strcmp(name, GetComponentName()) == 0 }
	static int IsComponentType2(lua_State* L) {
		Component& _p = *Lua::callerSharedPtr<Component>(L, 1);
		std::string name = Lua::get<std::string>(L, 2);
		return Lua::push(L, _p->IsComponentType(name.c_str()));
	}

	// virtual suede_guid GetComponentInstanceGUID() const { return GetComponentGUID() }
	static int GetComponentInstanceGUID(lua_State* L) {
		Component& _p = *Lua::callerSharedPtr<Component>(L, 0);
		return Lua::push(L, _p->GetComponentInstanceGUID());
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<Component>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& regs) {
		luaL_Reg metalib[] = {
			{ "__gc", Lua::deleteSharedPtr<Component> },
			{ "Awake", Awake },
			{ "OnRenderImage", OnRenderImage },
			{ "GetEnabled", GetEnabled },
			{ "SetEnabled", SetEnabled },
			{ "SetGameObject", SetGameObject },
			{ "GetGameObject", GetGameObject },
			{ "GetTransform", GetTransform },
			{ "CullingUpdate", CullingUpdate },
			{ "RenderingUpdate", RenderingUpdate },
			{ "GetUpdateStrategy", GetUpdateStrategy },
			{ "AllowMultiple", AllowMultiple },
			{ "IsComponentType", IsComponentType },
			{ "IsComponentType2", IsComponentType2 },
			{ "GetComponentInstanceGUID", GetComponentInstanceGUID },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<Component>(L, metalib, Lua::metatableName<Object>());
	}
};
