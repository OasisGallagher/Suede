// Warning: this file is generated by wrapper.py.

#pragma once

#include "gizmos.h"

#include "lua++.h"
#include "tools/string.h"

class Gizmos_Wrapper {
	static int ToString(lua_State* L) {
		Gizmos* _p = Lua::callerPtr<Gizmos>(L);

		lua_pushstring(L, String::Format("Gizmos@0x%p", _p).c_str());
		return 1;
	}

	static int ToStringStatic(lua_State* L) {
		lua_pushstring(L, "static Gizmos");
		return 1;
	}

	static int GizmosStatic(lua_State* L) {
		lua_newtable(L);

		luaL_Reg funcs[] = {
			{ "Flush", Flush },
			{ "GetMatrix", GetMatrix },
			{ "SetMatrix", SetMatrix },
			{ "GetColor", GetColor },
			{ "SetColor", SetColor },
			{ "DrawLines", DrawLines },
			{ "DrawSphere", DrawSphere },
			{ "DrawCuboid", DrawCuboid },
			{ "DrawCone", DrawCone },
			{ "DrawWireSphere", DrawWireSphere },
			{ "DrawWireCuboid", DrawWireCuboid },
			{"__tostring", ToStringStatic },
			{ nullptr, nullptr }
		};

		luaL_setfuncs(L, funcs, 0);

		return 1;
	}
	// static void Flush()
	static int Flush(lua_State* L) {
		Gizmos::Flush();
		return 0;
	}

	// static Matrix4 GetMatrix()
	static int GetMatrix(lua_State* L) {
		return Lua::push(L, Gizmos::GetMatrix());
	}

	// static void SetMatrix(const Matrix4& value)
	static int SetMatrix(lua_State* L) {
		Matrix4 value = Lua::get<Matrix4>(L, 1);
		
		Gizmos::SetMatrix(value);
		return 0;
	}

	// static Color GetColor()
	static int GetColor(lua_State* L) {
		return Lua::push(L, Gizmos::GetColor());
	}

	// static void SetColor(const Color& value)
	static int SetColor(lua_State* L) {
		Color value = Lua::get<Color>(L, 1);
		
		Gizmos::SetColor(value);
		return 0;
	}

	// static void DrawLines(const std::initializer_list<Vector3>& points)
	// static void DrawLines(const std::initializer_list<Vector3>& points, const std::initializer_list<uint>& indexes)
	static int DrawLines(lua_State* L) {
		if (Lua::checkArguments<std::initializer_list<Vector3>>(L, 2)) {
			std::initializer_list<Vector3> points = Lua::get<std::initializer_list<Vector3>>(L, 1);
			
			Gizmos::DrawLines(points);
			return 0;
		}

		if (Lua::checkArguments<std::initializer_list<Vector3>, std::initializer_list<uint>>(L, 2)) {
			std::initializer_list<uint> indexes = Lua::get<std::initializer_list<uint>>(L, 2);
			std::initializer_list<Vector3> points = Lua::get<std::initializer_list<Vector3>>(L, 1);
			
			Gizmos::DrawLines(points, indexes);
			return 0;
		}

		Debug::LogError("failed to call \"DrawLines\", invalid arguments.");
		return 0;
	}

	// static void DrawSphere(const Vector3& center, float radius)
	static int DrawSphere(lua_State* L) {
		float radius = Lua::get<float>(L, 2);
		Vector3 center = Lua::get<Vector3>(L, 1);
		
		Gizmos::DrawSphere(center, radius);
		return 0;
	}

	// static void DrawCuboid(const Vector3& center, const Vector3& size)
	static int DrawCuboid(lua_State* L) {
		Vector3 size = Lua::get<Vector3>(L, 2);
		Vector3 center = Lua::get<Vector3>(L, 1);
		
		Gizmos::DrawCuboid(center, size);
		return 0;
	}

	// static void DrawCone()
	static int DrawCone(lua_State* L) {
		Gizmos::DrawCone();
		return 0;
	}

	// static void DrawWireSphere(const Vector3& center, float radius)
	static int DrawWireSphere(lua_State* L) {
		float radius = Lua::get<float>(L, 2);
		Vector3 center = Lua::get<Vector3>(L, 1);
		
		Gizmos::DrawWireSphere(center, radius);
		return 0;
	}

	// static void DrawWireCuboid(const Vector3& center, const Vector3& size)
	static int DrawWireCuboid(lua_State* L) {
		Vector3 size = Lua::get<Vector3>(L, 2);
		Vector3 center = Lua::get<Vector3>(L, 1);
		
		Gizmos::DrawWireCuboid(center, size);
		return 0;
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<Gizmos>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		fields.push_back(luaL_Reg{ "Gizmos", GizmosStatic });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deletePtr<Gizmos> },
			{ "__tostring", ToString }, 
			{ nullptr, nullptr }
		};

		Lua::initMetatable<Gizmos>(L, metalib, nullptr);
	}
};
