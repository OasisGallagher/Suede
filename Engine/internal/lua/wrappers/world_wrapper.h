// Warning: this file is generated by wrapper.py.

#pragma once

#include "../luax.h"
#include "world.h"

class World_Wrapper {
	static int WorldInstance(lua_State* L) {
		return Lua::reference<World>(L);
	}

	static int Initialize(lua_State* L) {
		World* _p = World::instance();
		_p->Initialize();
		return 0;
	}

	static int CullingUpdate(lua_State* L) {
		World* _p = World::instance();
		_p->CullingUpdate();
		return 0;
	}

	static int RenderingUpdate(lua_State* L) {
		World* _p = World::instance();
		_p->RenderingUpdate();
		return 0;
	}

	static int Finalize(lua_State* L) {
		World* _p = World::instance();
		_p->Finalize();
		return 0;
	}

	static int CreateObject(lua_State* L) {
		World* _p = World::instance();
		ObjectType type = Lua::get<ObjectType>(L, -1);
		return Lua::push(L, _p->CreateObject(type));
	}

	static int DestroyGameObject(lua_State* L) {
		World* _p = World::instance();
		uint id = Lua::get<uint>(L, -1);
		_p->DestroyGameObject(id);
		return 0;
	}

	static int DestroyGameObject2(lua_State* L) {
		World* _p = World::instance();
		GameObject go = Lua::get<GameObject>(L, -1);
		_p->DestroyGameObject(go);
		return 0;
	}

	static int GetRootTransform(lua_State* L) {
		World* _p = World::instance();
		return Lua::push(L, _p->GetRootTransform());
	}

	static int GetGameObject(lua_State* L) {
		World* _p = World::instance();
		uint id = Lua::get<uint>(L, -1);
		return Lua::push(L, _p->GetGameObject(id));
	}

	static int FireEvent(lua_State* L) {
		World* _p = World::instance();
		WorldEventBasePtr e = Lua::get<WorldEventBasePtr>(L, -1);
		_p->FireEvent(e);
		return 0;
	}

	static int FireEventImmediate(lua_State* L) {
		World* _p = World::instance();
		WorldEventBasePtr e = Lua::get<WorldEventBasePtr>(L, -1);
		_p->FireEventImmediate(e);
		return 0;
	}

	static int GetDecals(lua_State* L) {
		World* _p = World::instance();
		std::vector<Decal> container = Lua::getList<Decal>(L, -1);
		_p->GetDecals(container);
		return 0;
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<World>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& regs) {
		regs.push_back(luaL_Reg { "WorldInstance", WorldInstance });

		luaL_Reg metalib[] = {
			{ "Initialize", Initialize },
			{ "CullingUpdate", CullingUpdate },
			{ "RenderingUpdate", RenderingUpdate },
			{ "Finalize", Finalize },
			{ "CreateObject", CreateObject },
			{ "DestroyGameObject", DestroyGameObject },
			{ "DestroyGameObject2", DestroyGameObject2 },
			{ "GetRootTransform", GetRootTransform },
			{ "GetGameObject", GetGameObject },
			{ "FireEvent", FireEvent },
			{ "FireEventImmediate", FireEventImmediate },
			{ "GetDecals", GetDecals },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<World>(L, metalib, nullptr);
	}
};