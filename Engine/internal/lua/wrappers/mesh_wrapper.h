// Warning: this file is generated by wrapper.py.

#pragma once

#include "../luax.h"
#include "mesh.h"

class SubMesh_Wrapper {
	static int NewSubMesh(lua_State* L) {
		return Lua::fromShared(L, ::NewSubMesh());
	}

	static int SetTriangleBias(lua_State* L) {
		SubMesh& _p = *Lua::callerSharedPtr<SubMesh>(L, 1);
		TriangleBias value = Lua::get<TriangleBias>(L, -1);
		_p->SetTriangleBias(value);
		return 0;
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<SubMesh>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& regs) {
		regs.push_back(luaL_Reg { "NewSubMesh", NewSubMesh });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deleteSharedPtr<SubMesh> },
			{ "SetTriangleBias", SetTriangleBias },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<SubMesh>(L, metalib, Lua::metatableName<Object>());
	}
};

class Mesh_Wrapper {
	static int NewMesh(lua_State* L) {
		return Lua::fromShared(L, ::NewMesh());
	}

	static int CreateStorage(lua_State* L) {
		Mesh& _p = *Lua::callerSharedPtr<Mesh>(L, 0);
		_p->CreateStorage();
		return 0;
	}

	static int SetAttribute(lua_State* L) {
		Mesh& _p = *Lua::callerSharedPtr<Mesh>(L, 1);
		MeshAttribute value = Lua::get<MeshAttribute>(L, -1);
		_p->SetAttribute(value);
		return 0;
	}

	static int SetBounds(lua_State* L) {
		Mesh& _p = *Lua::callerSharedPtr<Mesh>(L, 1);
		Bounds value = Lua::get<Bounds>(L, -1);
		_p->SetBounds(value);
		return 0;
	}

	static int AddSubMesh(lua_State* L) {
		Mesh& _p = *Lua::callerSharedPtr<Mesh>(L, 1);
		SubMesh subMesh = Lua::get<SubMesh>(L, -1);
		_p->AddSubMesh(subMesh);
		return 0;
	}

	static int GetSubMeshCount(lua_State* L) {
		Mesh& _p = *Lua::callerSharedPtr<Mesh>(L, 0);
		return Lua::push(L, _p->GetSubMeshCount());
	}

	static int GetSubMesh(lua_State* L) {
		Mesh& _p = *Lua::callerSharedPtr<Mesh>(L, 1);
		uint index = Lua::get<uint>(L, -1);
		return Lua::push(L, _p->GetSubMesh(index));
	}

	static int GetSubMeshes(lua_State* L) {
		Mesh& _p = *Lua::callerSharedPtr<Mesh>(L, 0);
		IMesh::Enumerable _r = _p->GetSubMeshes();
		return Lua::pushList(L, std::vector<IMesh::Enumerable::value_type>(_r.begin(), _r.end()));
	}

	static int RemoveSubMesh(lua_State* L) {
		Mesh& _p = *Lua::callerSharedPtr<Mesh>(L, 1);
		uint index = Lua::get<uint>(L, -1);
		_p->RemoveSubMesh(index);
		return 0;
	}

	static int GetTopology(lua_State* L) {
		Mesh& _p = *Lua::callerSharedPtr<Mesh>(L, 0);
		return Lua::push(L, _p->GetTopology());
	}

	static int GetNativePointer(lua_State* L) {
		Mesh& _p = *Lua::callerSharedPtr<Mesh>(L, 0);
		return Lua::push(L, _p->GetNativePointer());
	}

	static int UnmapIndexes(lua_State* L) {
		Mesh& _p = *Lua::callerSharedPtr<Mesh>(L, 0);
		_p->UnmapIndexes();
		return 0;
	}

	static int GetIndexCount(lua_State* L) {
		Mesh& _p = *Lua::callerSharedPtr<Mesh>(L, 0);
		return Lua::push(L, _p->GetIndexCount());
	}

	static int UnmapVertices(lua_State* L) {
		Mesh& _p = *Lua::callerSharedPtr<Mesh>(L, 0);
		_p->UnmapVertices();
		return 0;
	}

	static int GetVertexCount(lua_State* L) {
		Mesh& _p = *Lua::callerSharedPtr<Mesh>(L, 0);
		return Lua::push(L, _p->GetVertexCount());
	}

	static int Bind(lua_State* L) {
		Mesh& _p = *Lua::callerSharedPtr<Mesh>(L, 0);
		_p->Bind();
		return 0;
	}

	static int Unbind(lua_State* L) {
		Mesh& _p = *Lua::callerSharedPtr<Mesh>(L, 0);
		_p->Unbind();
		return 0;
	}

	static int ShareStorage(lua_State* L) {
		Mesh& _p = *Lua::callerSharedPtr<Mesh>(L, 1);
		Mesh other = Lua::get<Mesh>(L, -1);
		_p->ShareStorage(other);
		return 0;
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<Mesh>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& regs) {
		regs.push_back(luaL_Reg { "NewMesh", NewMesh });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deleteSharedPtr<Mesh> },
			{ "CreateStorage", CreateStorage },
			{ "SetAttribute", SetAttribute },
			{ "SetBounds", SetBounds },
			{ "AddSubMesh", AddSubMesh },
			{ "GetSubMeshCount", GetSubMeshCount },
			{ "GetSubMesh", GetSubMesh },
			{ "GetSubMeshes", GetSubMeshes },
			{ "RemoveSubMesh", RemoveSubMesh },
			{ "GetTopology", GetTopology },
			{ "GetNativePointer", GetNativePointer },
			{ "UnmapIndexes", UnmapIndexes },
			{ "GetIndexCount", GetIndexCount },
			{ "UnmapVertices", UnmapVertices },
			{ "GetVertexCount", GetVertexCount },
			{ "Bind", Bind },
			{ "Unbind", Unbind },
			{ "ShareStorage", ShareStorage },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<Mesh>(L, metalib, Lua::metatableName<Object>());
	}
};

class MeshProvider_Wrapper {
	static int GetMesh(lua_State* L) {
		MeshProvider& _p = *Lua::callerSharedPtr<MeshProvider>(L, 0);
		return Lua::push(L, _p->GetMesh());
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<MeshProvider>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& regs) {
		luaL_Reg metalib[] = {
			{ "__gc", Lua::deleteSharedPtr<MeshProvider> },
			{ "GetMesh", GetMesh },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<MeshProvider>(L, metalib, Lua::metatableName<Component>());
	}
};

class TextMesh_Wrapper {
	static int SetText(lua_State* L) {
		TextMesh& _p = *Lua::callerSharedPtr<TextMesh>(L, 1);
		std::string value = Lua::get<std::string>(L, -1);
		_p->SetText(value);
		return 0;
	}

	static int GetText(lua_State* L) {
		TextMesh& _p = *Lua::callerSharedPtr<TextMesh>(L, 0);
		return Lua::push(L, _p->GetText());
	}

	static int SetFont(lua_State* L) {
		TextMesh& _p = *Lua::callerSharedPtr<TextMesh>(L, 1);
		Font value = Lua::get<Font>(L, -1);
		_p->SetFont(value);
		return 0;
	}

	static int GetFont(lua_State* L) {
		TextMesh& _p = *Lua::callerSharedPtr<TextMesh>(L, 0);
		return Lua::push(L, _p->GetFont());
	}

	static int SetFontSize(lua_State* L) {
		TextMesh& _p = *Lua::callerSharedPtr<TextMesh>(L, 1);
		uint value = Lua::get<uint>(L, -1);
		_p->SetFontSize(value);
		return 0;
	}

	static int GetFontSize(lua_State* L) {
		TextMesh& _p = *Lua::callerSharedPtr<TextMesh>(L, 0);
		return Lua::push(L, _p->GetFontSize());
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<TextMesh>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& regs) {
		luaL_Reg metalib[] = {
			{ "__gc", Lua::deleteSharedPtr<TextMesh> },
			{ "SetText", SetText },
			{ "GetText", GetText },
			{ "SetFont", SetFont },
			{ "GetFont", GetFont },
			{ "SetFontSize", SetFontSize },
			{ "GetFontSize", GetFontSize },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<TextMesh>(L, metalib, Lua::metatableName<MeshProvider>());
	}
};

class MeshFilter_Wrapper {
	static int SetMesh(lua_State* L) {
		MeshFilter& _p = *Lua::callerSharedPtr<MeshFilter>(L, 1);
		Mesh value = Lua::get<Mesh>(L, -1);
		_p->SetMesh(value);
		return 0;
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<MeshFilter>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& regs) {
		luaL_Reg metalib[] = {
			{ "__gc", Lua::deleteSharedPtr<MeshFilter> },
			{ "SetMesh", SetMesh },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<MeshFilter>(L, metalib, Lua::metatableName<MeshProvider>());
	}
};