// Warning: this file is generated by wrapper.py.

#pragma once

#include "material.h"

#include "lua++.h"
#include "tools/string.h"

class Material_Wrapper {
	static int NewMaterial(lua_State* L) {
		return Lua::newObject<Material>(L);
	}

	static int ToString(lua_State* L) {
		Material* _p = Lua::callerPtr<Material>(L);

		lua_pushstring(L, String::Format("Material@0x%p", _p).c_str());
		return 1;
	}

	// ref_ptr<Object> Clone()
	static int Clone(lua_State* L) {
		Material* _p = Lua::callerPtr<Material>(L);
		return Lua::push(L, _p->Clone());
	}

	// void Bind(uint pass)
	static int Bind(lua_State* L) {
		Material* _p = Lua::callerPtr<Material>(L);
		uint pass = Lua::get<uint>(L, 2);
		
		_p->Bind(pass);
		return 0;
	}

	// void Unbind()
	static int Unbind(lua_State* L) {
		Material* _p = Lua::callerPtr<Material>(L);
		_p->Unbind();
		return 0;
	}

	// bool EnablePass(uint pass)
	static int EnablePass(lua_State* L) {
		Material* _p = Lua::callerPtr<Material>(L);
		uint pass = Lua::get<uint>(L, 2);
		
		return Lua::push(L, _p->EnablePass(pass));
	}

	// bool DisablePass(uint pass)
	static int DisablePass(lua_State* L) {
		Material* _p = Lua::callerPtr<Material>(L);
		uint pass = Lua::get<uint>(L, 2);
		
		return Lua::push(L, _p->DisablePass(pass));
	}

	// bool IsPassEnabled(uint pass)
	static int IsPassEnabled(lua_State* L) {
		Material* _p = Lua::callerPtr<Material>(L);
		uint pass = Lua::get<uint>(L, 2);
		
		return Lua::push(L, _p->IsPassEnabled(pass));
	}

	// int FindPass(const std::string& name)
	static int FindPass(lua_State* L) {
		Material* _p = Lua::callerPtr<Material>(L);
		std::string name = Lua::get<std::string>(L, 2);
		
		return Lua::push(L, _p->FindPass(name));
	}

	// void SetPass(int pass)
	static int SetPass(lua_State* L) {
		Material* _p = Lua::callerPtr<Material>(L);
		int pass = Lua::get<int>(L, 2);
		
		_p->SetPass(pass);
		return 0;
	}

	// int GetPass()
	static int GetPass(lua_State* L) {
		Material* _p = Lua::callerPtr<Material>(L);
		return Lua::push(L, _p->GetPass());
	}

	// uint GetPassCount()
	static int GetPassCount(lua_State* L) {
		Material* _p = Lua::callerPtr<Material>(L);
		return Lua::push(L, _p->GetPassCount());
	}

	// uint GetPassNativePointer(uint pass)
	static int GetPassNativePointer(lua_State* L) {
		Material* _p = Lua::callerPtr<Material>(L);
		uint pass = Lua::get<uint>(L, 2);
		
		return Lua::push(L, _p->GetPassNativePointer(pass));
	}

	// void SetRenderQueue(int value)
	static int SetRenderQueue(lua_State* L) {
		Material* _p = Lua::callerPtr<Material>(L);
		int value = Lua::get<int>(L, 2);
		
		_p->SetRenderQueue(value);
		return 0;
	}

	// int GetRenderQueue()
	static int GetRenderQueue(lua_State* L) {
		Material* _p = Lua::callerPtr<Material>(L);
		return Lua::push(L, _p->GetRenderQueue());
	}

	// void Define(const std::string& name)
	static int Define(lua_State* L) {
		Material* _p = Lua::callerPtr<Material>(L);
		std::string name = Lua::get<std::string>(L, 2);
		
		_p->Define(name);
		return 0;
	}

	// void Undefine(const std::string& name)
	static int Undefine(lua_State* L) {
		Material* _p = Lua::callerPtr<Material>(L);
		std::string name = Lua::get<std::string>(L, 2);
		
		_p->Undefine(name);
		return 0;
	}

	// bool HasProperty(const std::string& name)
	static int HasProperty(lua_State* L) {
		Material* _p = Lua::callerPtr<Material>(L);
		std::string name = Lua::get<std::string>(L, 2);
		
		return Lua::push(L, _p->HasProperty(name));
	}

	// void SetInt(const std::string& name, int value)
	static int SetInt(lua_State* L) {
		Material* _p = Lua::callerPtr<Material>(L);
		int value = Lua::get<int>(L, 3);
		std::string name = Lua::get<std::string>(L, 2);
		
		_p->SetInt(name, value);
		return 0;
	}

	// void SetBool(const std::string& name, bool value)
	static int SetBool(lua_State* L) {
		Material* _p = Lua::callerPtr<Material>(L);
		bool value = Lua::get<bool>(L, 3);
		std::string name = Lua::get<std::string>(L, 2);
		
		_p->SetBool(name, value);
		return 0;
	}

	// void SetFloat(const std::string& name, float value)
	static int SetFloat(lua_State* L) {
		Material* _p = Lua::callerPtr<Material>(L);
		float value = Lua::get<float>(L, 3);
		std::string name = Lua::get<std::string>(L, 2);
		
		_p->SetFloat(name, value);
		return 0;
	}

	// void SetMatrix4(const std::string& name, const Matrix4& value)
	static int SetMatrix4(lua_State* L) {
		Material* _p = Lua::callerPtr<Material>(L);
		Matrix4 value = Lua::get<Matrix4>(L, 3);
		std::string name = Lua::get<std::string>(L, 2);
		
		_p->SetMatrix4(name, value);
		return 0;
	}

	// void SetVector3(const std::string& name, const Vector3& value)
	static int SetVector3(lua_State* L) {
		Material* _p = Lua::callerPtr<Material>(L);
		Vector3 value = Lua::get<Vector3>(L, 3);
		std::string name = Lua::get<std::string>(L, 2);
		
		_p->SetVector3(name, value);
		return 0;
	}

	// void SetColor(const std::string& name, const Color& value)
	static int SetColor(lua_State* L) {
		Material* _p = Lua::callerPtr<Material>(L);
		Color value = Lua::get<Color>(L, 3);
		std::string name = Lua::get<std::string>(L, 2);
		
		_p->SetColor(name, value);
		return 0;
	}

	// void SetVector4(const std::string& name, const Vector4& value)
	static int SetVector4(lua_State* L) {
		Material* _p = Lua::callerPtr<Material>(L);
		Vector4 value = Lua::get<Vector4>(L, 3);
		std::string name = Lua::get<std::string>(L, 2);
		
		_p->SetVector4(name, value);
		return 0;
	}

	// void SetVariant(const std::string& name, const Variant& value)
	static int SetVariant(lua_State* L) {
		Material* _p = Lua::callerPtr<Material>(L);
		Variant value = Lua::get<Variant>(L, 3);
		std::string name = Lua::get<std::string>(L, 2);
		
		_p->SetVariant(name, value);
		return 0;
	}

	// int GetInt(const std::string& name)
	static int GetInt(lua_State* L) {
		Material* _p = Lua::callerPtr<Material>(L);
		std::string name = Lua::get<std::string>(L, 2);
		
		return Lua::push(L, _p->GetInt(name));
	}

	// bool GetBool(const std::string& name)
	static int GetBool(lua_State* L) {
		Material* _p = Lua::callerPtr<Material>(L);
		std::string name = Lua::get<std::string>(L, 2);
		
		return Lua::push(L, _p->GetBool(name));
	}

	// float GetFloat(const std::string& name)
	static int GetFloat(lua_State* L) {
		Material* _p = Lua::callerPtr<Material>(L);
		std::string name = Lua::get<std::string>(L, 2);
		
		return Lua::push(L, _p->GetFloat(name));
	}

	// iranged GetRangedInt(const std::string& name)
	static int GetRangedInt(lua_State* L) {
		Material* _p = Lua::callerPtr<Material>(L);
		std::string name = Lua::get<std::string>(L, 2);
		
		return Lua::push(L, _p->GetRangedInt(name));
	}

	// franged GetRangedFloat(const std::string& name)
	static int GetRangedFloat(lua_State* L) {
		Material* _p = Lua::callerPtr<Material>(L);
		std::string name = Lua::get<std::string>(L, 2);
		
		return Lua::push(L, _p->GetRangedFloat(name));
	}

	// Matrix4 GetMatrix4(const std::string& name)
	static int GetMatrix4(lua_State* L) {
		Material* _p = Lua::callerPtr<Material>(L);
		std::string name = Lua::get<std::string>(L, 2);
		
		return Lua::push(L, _p->GetMatrix4(name));
	}

	// Vector3 GetVector3(const std::string& name)
	static int GetVector3(lua_State* L) {
		Material* _p = Lua::callerPtr<Material>(L);
		std::string name = Lua::get<std::string>(L, 2);
		
		return Lua::push(L, _p->GetVector3(name));
	}

	// Color GetColor(const std::string& name)
	static int GetColor(lua_State* L) {
		Material* _p = Lua::callerPtr<Material>(L);
		std::string name = Lua::get<std::string>(L, 2);
		
		return Lua::push(L, _p->GetColor(name));
	}

	// Vector4 GetVector4(const std::string& name)
	static int GetVector4(lua_State* L) {
		Material* _p = Lua::callerPtr<Material>(L);
		std::string name = Lua::get<std::string>(L, 2);
		
		return Lua::push(L, _p->GetVector4(name));
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<Material>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		funcs.push_back(luaL_Reg { "NewMaterial", NewMaterial });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deletePtr<Material> },
			{ "__tostring", ToString }, 
			{ "Clone", Clone },
			{ "Bind", Bind },
			{ "Unbind", Unbind },
			{ "EnablePass", EnablePass },
			{ "DisablePass", DisablePass },
			{ "IsPassEnabled", IsPassEnabled },
			{ "FindPass", FindPass },
			{ "SetPass", SetPass },
			{ "GetPass", GetPass },
			{ "GetPassCount", GetPassCount },
			{ "GetPassNativePointer", GetPassNativePointer },
			{ "SetRenderQueue", SetRenderQueue },
			{ "GetRenderQueue", GetRenderQueue },
			{ "Define", Define },
			{ "Undefine", Undefine },
			{ "HasProperty", HasProperty },
			{ "SetInt", SetInt },
			{ "SetBool", SetBool },
			{ "SetFloat", SetFloat },
			{ "SetMatrix4", SetMatrix4 },
			{ "SetVector3", SetVector3 },
			{ "SetColor", SetColor },
			{ "SetVector4", SetVector4 },
			{ "SetVariant", SetVariant },
			{ "GetInt", GetInt },
			{ "GetBool", GetBool },
			{ "GetFloat", GetFloat },
			{ "GetRangedInt", GetRangedInt },
			{ "GetRangedFloat", GetRangedFloat },
			{ "GetMatrix4", GetMatrix4 },
			{ "GetVector3", GetVector3 },
			{ "GetColor", GetColor },
			{ "GetVector4", GetVector4 },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<Material>(L, metalib, TypeID<Object>::string());
	}
};
