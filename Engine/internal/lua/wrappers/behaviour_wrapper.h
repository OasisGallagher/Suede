// Warning: this file is generated by wrapper.py.

#pragma once

#include "lua++.h"
#include "behaviour.h"

class Behaviour_Wrapper {
	static int NewBehaviour(lua_State* L) {
		return Lua::newObject<Behaviour>(L);
	}

	static int Awake(lua_State* L) {
		Behaviour* _p = Lua::callerPtr<Behaviour>(L, 0);
		_p->Awake();
		return 0;
	}

	static int Update(lua_State* L) {
		Behaviour* _p = Lua::callerPtr<Behaviour>(L, 0);
		_p->Update();
		return 0;
	}

	static int OnRenderImage(lua_State* L) {
		Behaviour* _p = Lua::callerPtr<Behaviour>(L, 3);
		Rect normalizedRect = Lua::get<Rect>(L, 4);
		RenderTexture dest = Lua::get<RenderTexture>(L, 3);
		RenderTexture src = Lua::get<RenderTexture>(L, 2);
		_p->OnRenderImage(src, dest, normalizedRect);
		return 0;
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<Behaviour>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& regs) {
		regs.push_back(luaL_Reg { "NewBehaviour", NewBehaviour });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deletePtr<Behaviour> },
			{ "Awake", Awake },
			{ "Update", Update },
			{ "OnRenderImage", OnRenderImage },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<Behaviour>(L, metalib, Lua::metatableName<Component>());
	}
};
