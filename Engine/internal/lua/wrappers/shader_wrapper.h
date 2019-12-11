// Warning: this file is generated by wrapper.py.

#pragma once

#include "shader.h"

#include "lua++.h"
#include "tools/string.h"

class Property_Wrapper {
	static int NewProperty(lua_State* L) {
		return Lua::newObject<Property>(L);
	}

	static int ToString(lua_State* L) {
		Property* _p = Lua::callerPtr<Property>(L);

		lua_pushstring(L, String::Format("Property@0x%p", _p).c_str());
		return 1;
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<Property>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		funcs.push_back(luaL_Reg { "NewProperty", NewProperty });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deletePtr<Property> },
			{ "__tostring", ToString }, 
			{ nullptr, nullptr }
		};

		Lua::initMetatable<Property>(L, metalib, nullptr);
	}
};

class ShaderProperty_Wrapper {
	static int NewShaderProperty(lua_State* L) {
		return Lua::newObject<ShaderProperty>(L);
	}

	static int ToString(lua_State* L) {
		ShaderProperty* _p = Lua::callerPtr<ShaderProperty>(L);

		lua_pushstring(L, String::Format("ShaderProperty@0x%p", _p).c_str());
		return 1;
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<ShaderProperty>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		funcs.push_back(luaL_Reg { "NewShaderProperty", NewShaderProperty });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deletePtr<ShaderProperty> },
			{ "__tostring", ToString }, 
			{ nullptr, nullptr }
		};

		Lua::initMetatable<ShaderProperty>(L, metalib, nullptr);
	}
};

class Shader_Wrapper {
	static int NewShader(lua_State* L) {
		return Lua::newObject<Shader>(L);
	}

	static int ToString(lua_State* L) {
		Shader* _p = Lua::callerPtr<Shader>(L);

		lua_pushstring(L, String::Format("Shader@0x%p", _p).c_str());
		return 1;
	}

	// std::string GetName()
	static int GetName(lua_State* L) {
		Shader* _p = Lua::callerPtr<Shader>(L);
		return Lua::push(L, _p->GetName());
	}

	// bool Load(const std::string& path)
	static int Load(lua_State* L) {
		Shader* _p = Lua::callerPtr<Shader>(L);
		std::string path = Lua::get<std::string>(L, 2);
		
		return Lua::push(L, _p->Load(path));
	}

	// void Bind(uint ssi, uint pass)
	static int Bind(lua_State* L) {
		Shader* _p = Lua::callerPtr<Shader>(L);
		uint pass = Lua::get<uint>(L, 3);
		uint ssi = Lua::get<uint>(L, 2);
		
		_p->Bind(ssi, pass);
		return 0;
	}

	// void Unbind()
	static int Unbind(lua_State* L) {
		Shader* _p = Lua::callerPtr<Shader>(L);
		_p->Unbind();
		return 0;
	}

	// void SetRenderQueue(uint ssi, int value)
	static int SetRenderQueue(lua_State* L) {
		Shader* _p = Lua::callerPtr<Shader>(L);
		int value = Lua::get<int>(L, 3);
		uint ssi = Lua::get<uint>(L, 2);
		
		_p->SetRenderQueue(ssi, value);
		return 0;
	}

	// int GetRenderQueue(uint ssi)
	static int GetRenderQueue(lua_State* L) {
		Shader* _p = Lua::callerPtr<Shader>(L);
		uint ssi = Lua::get<uint>(L, 2);
		
		return Lua::push(L, _p->GetRenderQueue(ssi));
	}

	// bool IsPassEnabled(uint ssi, uint pass)
	static int IsPassEnabled(lua_State* L) {
		Shader* _p = Lua::callerPtr<Shader>(L);
		uint pass = Lua::get<uint>(L, 3);
		uint ssi = Lua::get<uint>(L, 2);
		
		return Lua::push(L, _p->IsPassEnabled(ssi, pass));
	}

	// int GetPassIndex(uint ssi, const std::string& name)
	static int GetPassIndex(lua_State* L) {
		Shader* _p = Lua::callerPtr<Shader>(L);
		std::string name = Lua::get<std::string>(L, 3);
		uint ssi = Lua::get<uint>(L, 2);
		
		return Lua::push(L, _p->GetPassIndex(ssi, name));
	}

	// uint GetNativePointer(uint ssi, uint pass)
	static int GetNativePointer(lua_State* L) {
		Shader* _p = Lua::callerPtr<Shader>(L);
		uint pass = Lua::get<uint>(L, 3);
		uint ssi = Lua::get<uint>(L, 2);
		
		return Lua::push(L, _p->GetNativePointer(ssi, pass));
	}

	// uint GetPassCount(uint ssi)
	static int GetPassCount(lua_State* L) {
		Shader* _p = Lua::callerPtr<Shader>(L);
		uint ssi = Lua::get<uint>(L, 2);
		
		return Lua::push(L, _p->GetPassCount(ssi));
	}

	// uint GetSubShaderCount()
	static int GetSubShaderCount(lua_State* L) {
		Shader* _p = Lua::callerPtr<Shader>(L);
		return Lua::push(L, _p->GetSubShaderCount());
	}

	// void GetProperties(std::vector<ShaderProperty>& properties)
	static int GetProperties(lua_State* L) {
		Shader* _p = Lua::callerPtr<Shader>(L);
		std::vector<ShaderProperty> properties = Lua::getList<ShaderProperty>(L, 2);
		
		_p->GetProperties(properties);
		return 0;
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<Shader>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		funcs.push_back(luaL_Reg { "NewShader", NewShader });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deletePtr<Shader> },
			{ "__tostring", ToString }, 
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

		Lua::initMetatable<Shader>(L, metalib, TypeID<Object>::string());
	}
};
