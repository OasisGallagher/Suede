// Warning: this file is generated by wrapper.py.

#pragma once

#include "../luax.h"
#include "shader.h"

class Shader_Wrapper {
	static int NewShader(lua_State* L) {
		return Lua::fromShared(L, ::NewShader());
	}

	static int GetName(lua_State* L) {
		Shader& _p = *Lua::callerSharedPtr<Shader>(L, 0);
		return Lua::push(L, _p->GetName());
	}

	static int Load(lua_State* L) {
		Shader& _p = *Lua::callerSharedPtr<Shader>(L, 1);
		std::string path = Lua::get<std::string>(L, -1);
		return Lua::push(L, _p->Load(path));
	}

	static int Bind(lua_State* L) {
		Shader& _p = *Lua::callerSharedPtr<Shader>(L, 2);
		uint ssi = Lua::get<uint>(L, -1);
		uint pass = Lua::get<uint>(L, -2);
		_p->Bind(ssi, pass);
		return 0;
	}

	static int Unbind(lua_State* L) {
		Shader& _p = *Lua::callerSharedPtr<Shader>(L, 0);
		_p->Unbind();
		return 0;
	}

	static int SetRenderQueue(lua_State* L) {
		Shader& _p = *Lua::callerSharedPtr<Shader>(L, 2);
		uint ssi = Lua::get<uint>(L, -1);
		int value = Lua::get<int>(L, -2);
		_p->SetRenderQueue(ssi, value);
		return 0;
	}

	static int GetRenderQueue(lua_State* L) {
		Shader& _p = *Lua::callerSharedPtr<Shader>(L, 1);
		uint ssi = Lua::get<uint>(L, -1);
		return Lua::push(L, _p->GetRenderQueue(ssi));
	}

	static int IsPassEnabled(lua_State* L) {
		Shader& _p = *Lua::callerSharedPtr<Shader>(L, 2);
		uint ssi = Lua::get<uint>(L, -1);
		uint pass = Lua::get<uint>(L, -2);
		return Lua::push(L, _p->IsPassEnabled(ssi, pass));
	}

	static int GetPassIndex(lua_State* L) {
		Shader& _p = *Lua::callerSharedPtr<Shader>(L, 2);
		uint ssi = Lua::get<uint>(L, -1);
		std::string name = Lua::get<std::string>(L, -2);
		return Lua::push(L, _p->GetPassIndex(ssi, name));
	}

	static int GetNativePointer(lua_State* L) {
		Shader& _p = *Lua::callerSharedPtr<Shader>(L, 2);
		uint ssi = Lua::get<uint>(L, -1);
		uint pass = Lua::get<uint>(L, -2);
		return Lua::push(L, _p->GetNativePointer(ssi, pass));
	}

	static int GetPassCount(lua_State* L) {
		Shader& _p = *Lua::callerSharedPtr<Shader>(L, 1);
		uint ssi = Lua::get<uint>(L, -1);
		return Lua::push(L, _p->GetPassCount(ssi));
	}

	static int GetSubShaderCount(lua_State* L) {
		Shader& _p = *Lua::callerSharedPtr<Shader>(L, 0);
		return Lua::push(L, _p->GetSubShaderCount());
	}

	static int GetProperties(lua_State* L) {
		Shader& _p = *Lua::callerSharedPtr<Shader>(L, 1);
		std::vector<ShaderProperty> properties = Lua::getList<ShaderProperty>(L, -1);
		_p->GetProperties(properties);
		return 0;
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<Shader>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& regs) {
		regs.push_back(luaL_Reg { "NewShader", NewShader });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deleteSharedPtr<Shader> },
			{ "GetName", GetName },
			{ "Load", Load },
			{ "Bind", Bind },
			{ "Unbind", Unbind },
			{ "SetRenderQueue", SetRenderQueue },
			{ "GetRenderQueue", GetRenderQueue },
			{ "IsPassEnabled", IsPassEnabled },
			{ "GetPassIndex", GetPassIndex },
			{ "GetNativePointer", GetNativePointer },
			{ "GetPassCount", GetPassCount },
			{ "GetSubShaderCount", GetSubShaderCount },
			{ "GetProperties", GetProperties },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<Shader>(L, metalib, Lua::metatableName<Object>());
	}
};
