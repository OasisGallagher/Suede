// Warning: this file is generated by wrapper.py.

#pragma once

#include "lua++.h"
#include "font.h"

class Font_Wrapper {
	static int NewFont(lua_State* L) {
		return Lua::fromShared(L, ::NewFont());
	}

	// bool Load(const std::string& path, int size)
	static int Load(lua_State* L) {
		Font& _p = *Lua::callerSharedPtr<Font>(L, 2);
		int size = Lua::get<int>(L, 3);
		std::string path = Lua::get<std::string>(L, 2);
		return Lua::push(L, _p->Load(path, size));
	}

	// bool Require(const std::wstring& str)
	static int Require(lua_State* L) {
		Font& _p = *Lua::callerSharedPtr<Font>(L, 1);
		std::wstring str = Lua::get<std::wstring>(L, 2);
		return Lua::push(L, _p->Require(str));
	}

	// uint GetFontSize() const
	static int GetFontSize(lua_State* L) {
		Font& _p = *Lua::callerSharedPtr<Font>(L, 0);
		return Lua::push(L, _p->GetFontSize());
	}

	// Texture2D GetTexture() const
	static int GetTexture(lua_State* L) {
		Font& _p = *Lua::callerSharedPtr<Font>(L, 0);
		return Lua::push(L, _p->GetTexture());
	}

	// std::string GetFamilyName() const
	static int GetFamilyName(lua_State* L) {
		Font& _p = *Lua::callerSharedPtr<Font>(L, 0);
		return Lua::push(L, _p->GetFamilyName());
	}

	// std::string GetStyleName() const
	static int GetStyleName(lua_State* L) {
		Font& _p = *Lua::callerSharedPtr<Font>(L, 0);
		return Lua::push(L, _p->GetStyleName());
	}

	// Material GetMaterial()
	static int GetMaterial(lua_State* L) {
		Font& _p = *Lua::callerSharedPtr<Font>(L, 0);
		return Lua::push(L, _p->GetMaterial());
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<Font>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& regs) {
		regs.push_back(luaL_Reg { "NewFont", NewFont });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deleteSharedPtr<Font> },
			{ "Load", Load },
			{ "Require", Require },
			{ "GetFontSize", GetFontSize },
			{ "GetTexture", GetTexture },
			{ "GetFamilyName", GetFamilyName },
			{ "GetStyleName", GetStyleName },
			{ "GetMaterial", GetMaterial },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<Font>(L, metalib, Lua::metatableName<Object>());
	}
};
