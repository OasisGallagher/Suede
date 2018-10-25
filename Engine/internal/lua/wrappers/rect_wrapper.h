// Warning: this file is generated by wrapper.py.

#pragma once

#include "lua++.h"
#include "rect.h"

class Rect_Wrapper {
	static int NewRect(lua_State* L) {
		return Lua::newObject<Rect>(L);
	}

	// glm::vec2 GetLeftTop() const
	static int GetLeftTop(lua_State* L) {
		Rect* _p = Lua::callerPtr<Rect>(L, 0);
		return Lua::push(L, _p->GetLeftTop());
	}

	// glm::vec2 GetLeftBottom() const
	static int GetLeftBottom(lua_State* L) {
		Rect* _p = Lua::callerPtr<Rect>(L, 0);
		return Lua::push(L, _p->GetLeftBottom());
	}

	// glm::vec2 GetRightTop() const
	static int GetRightTop(lua_State* L) {
		Rect* _p = Lua::callerPtr<Rect>(L, 0);
		return Lua::push(L, _p->GetRightTop());
	}

	// glm::vec2 GetRightBottom() const
	static int GetRightBottom(lua_State* L) {
		Rect* _p = Lua::callerPtr<Rect>(L, 0);
		return Lua::push(L, _p->GetRightBottom());
	}

	// void SetWidth(float value)
	static int SetWidth(lua_State* L) {
		Rect* _p = Lua::callerPtr<Rect>(L, 1);
		float value = Lua::get<float>(L, 2);
		_p->SetWidth(value);
		return 0;
	}

	// float GetWidth() const
	static int GetWidth(lua_State* L) {
		Rect* _p = Lua::callerPtr<Rect>(L, 0);
		return Lua::push(L, _p->GetWidth());
	}

	// void SetHeight(float value)
	static int SetHeight(lua_State* L) {
		Rect* _p = Lua::callerPtr<Rect>(L, 1);
		float value = Lua::get<float>(L, 2);
		_p->SetHeight(value);
		return 0;
	}

	// float GetHeight() const
	static int GetHeight(lua_State* L) {
		Rect* _p = Lua::callerPtr<Rect>(L, 0);
		return Lua::push(L, _p->GetHeight());
	}

	// void SetXMin(float value)
	static int SetXMin(lua_State* L) {
		Rect* _p = Lua::callerPtr<Rect>(L, 1);
		float value = Lua::get<float>(L, 2);
		_p->SetXMin(value);
		return 0;
	}

	// float GetXMin() const
	static int GetXMin(lua_State* L) {
		Rect* _p = Lua::callerPtr<Rect>(L, 0);
		return Lua::push(L, _p->GetXMin());
	}

	// void SetYMin(float value)
	static int SetYMin(lua_State* L) {
		Rect* _p = Lua::callerPtr<Rect>(L, 1);
		float value = Lua::get<float>(L, 2);
		_p->SetYMin(value);
		return 0;
	}

	// float GetYMin() const
	static int GetYMin(lua_State* L) {
		Rect* _p = Lua::callerPtr<Rect>(L, 0);
		return Lua::push(L, _p->GetYMin());
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<Rect>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& regs) {
		regs.push_back(luaL_Reg { "NewRect", NewRect });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deletePtr<Rect> },
			{ "GetLeftTop", GetLeftTop },
			{ "GetLeftBottom", GetLeftBottom },
			{ "GetRightTop", GetRightTop },
			{ "GetRightBottom", GetRightBottom },
			{ "SetWidth", SetWidth },
			{ "GetWidth", GetWidth },
			{ "SetHeight", SetHeight },
			{ "GetHeight", GetHeight },
			{ "SetXMin", SetXMin },
			{ "GetXMin", GetXMin },
			{ "SetYMin", SetYMin },
			{ "GetYMin", GetYMin },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<Rect>(L, metalib, nullptr);
	}
};
