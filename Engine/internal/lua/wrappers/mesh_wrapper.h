// Warning: this file is generated by wrapper.py.

#pragma once

#include "mesh.h"

#include "lua++.h"
#include "tools/string.h"

class SubMesh_Wrapper {
	static int NewSubMesh(lua_State* L) {
		return Lua::fromShared(L, ::NewSubMesh());
	}

	static int ToString(lua_State* L) {
		SubMesh& _p = *Lua::callerSharedPtr<SubMesh>(L, 0);
		lua_pushstring(L, String::Format("SubMesh@0x%p", _p.get()).c_str());
		return 1;
	}

	// void SetTriangleBias(const TriangleBias& value)
	static int SetTriangleBias(lua_State* L) {
		SubMesh& _p = *Lua::callerSharedPtr<SubMesh>(L, 1);
		TriangleBias value = Lua::get<TriangleBias>(L, 2);
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
			{ "__tostring", ToString }, 
			{ "SetTriangleBias", SetTriangleBias },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<SubMesh>(L, metalib, TypeID<Object>::name());
	}
};

class Mesh_Wrapper {
	static int NewMesh(lua_State* L) {
		return Lua::fromShared(L, ::NewMesh());
	}

	static int ToString(lua_State* L) {
		Mesh& _p = *Lua::callerSharedPtr<Mesh>(L, 0);
		lua_pushstring(L, String::Format("Mesh@0x%p", _p.get()).c_str());
		return 1;
	}

	// void CreateStorage()
	static int CreateStorage(lua_State* L) {
		Mesh& _p = *Lua::callerSharedPtr<Mesh>(L, 0);
		_p->CreateStorage();
		return 0;
	}

	// void SetAttribute(const MeshAttribute& value)
	static int SetAttribute(lua_State* L) {
		Mesh& _p = *Lua::callerSharedPtr<Mesh>(L, 1);
		MeshAttribute value = Lua::get<MeshAttribute>(L, 2);
		_p->SetAttribute(value);
		return 0;
	}

	// void SetBounds(const Bounds& value)
	static int SetBounds(lua_State* L) {
		Mesh& _p = *Lua::callerSharedPtr<Mesh>(L, 1);
		Bounds value = Lua::get<Bounds>(L, 2);
		_p->SetBounds(value);
		return 0;
	}

	// void AddSubMesh(SubMesh subMesh)
	static int AddSubMesh(lua_State* L) {
		Mesh& _p = *Lua::callerSharedPtr<Mesh>(L, 1);
		SubMesh subMesh = Lua::get<SubMesh>(L, 2);
		_p->AddSubMesh(subMesh);
		return 0;
	}

	// int GetSubMeshCount()
	static int GetSubMeshCount(lua_State* L) {
		Mesh& _p = *Lua::callerSharedPtr<Mesh>(L, 0);
		return Lua::push(L, _p->GetSubMeshCount());
	}

	// SubMesh GetSubMesh(uint index)
	static int GetSubMesh(lua_State* L) {
		Mesh& _p = *Lua::callerSharedPtr<Mesh>(L, 1);
		uint index = Lua::get<uint>(L, 2);
		return Lua::push(L, _p->GetSubMesh(index));
	}

	// Enumerable GetSubMeshes()
	static int GetSubMeshes(lua_State* L) {
		Mesh& _p = *Lua::callerSharedPtr<Mesh>(L, 0);
		IMesh::Enumerable _r = _p->GetSubMeshes();
		return Lua::pushList(L, std::vector<IMesh::Enumerable::value_type>(_r.begin(), _r.end()));
	}

	// void RemoveSubMesh(uint index)
	static int RemoveSubMesh(lua_State* L) {
		Mesh& _p = *Lua::callerSharedPtr<Mesh>(L, 1);
		uint index = Lua::get<uint>(L, 2);
		_p->RemoveSubMesh(index);
		return 0;
	}

	// MeshTopology GetTopology()
	static int GetTopology(lua_State* L) {
		Mesh& _p = *Lua::callerSharedPtr<Mesh>(L, 0);
		return Lua::push(L, _p->GetTopology());
	}

	// uint GetNativePointer()
	static int GetNativePointer(lua_State* L) {
		Mesh& _p = *Lua::callerSharedPtr<Mesh>(L, 0);
		return Lua::push(L, _p->GetNativePointer());
	}

	// void UnmapIndexes()
	static int UnmapIndexes(lua_State* L) {
		Mesh& _p = *Lua::callerSharedPtr<Mesh>(L, 0);
		_p->UnmapIndexes();
		return 0;
	}

	// uint GetIndexCount()
	static int GetIndexCount(lua_State* L) {
		Mesh& _p = *Lua::callerSharedPtr<Mesh>(L, 0);
		return Lua::push(L, _p->GetIndexCount());
	}

	// void UnmapVertices()
	static int UnmapVertices(lua_State* L) {
		Mesh& _p = *Lua::callerSharedPtr<Mesh>(L, 0);
		_p->UnmapVertices();
		return 0;
	}

	// uint GetVertexCount()
	static int GetVertexCount(lua_State* L) {
		Mesh& _p = *Lua::callerSharedPtr<Mesh>(L, 0);
		return Lua::push(L, _p->GetVertexCount());
	}

	// void Bind()
	static int Bind(lua_State* L) {
		Mesh& _p = *Lua::callerSharedPtr<Mesh>(L, 0);
		_p->Bind();
		return 0;
	}

	// void Unbind()
	static int Unbind(lua_State* L) {
		Mesh& _p = *Lua::callerSharedPtr<Mesh>(L, 0);
		_p->Unbind();
		return 0;
	}

	// void ShareStorage(Mesh other)
	static int ShareStorage(lua_State* L) {
		Mesh& _p = *Lua::callerSharedPtr<Mesh>(L, 1);
		Mesh other = Lua::get<Mesh>(L, 2);
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
			{ "__tostring", ToString }, 
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

		Lua::initMetatable<Mesh>(L, metalib, TypeID<Object>::name());
	}
};

class MeshProvider_Wrapper {
	static int ToString(lua_State* L) {
		MeshProvider& _p = *Lua::callerSharedPtr<MeshProvider>(L, 0);
		lua_pushstring(L, String::Format("MeshProvider@0x%p", _p.get()).c_str());
		return 1;
	}

	// Mesh GetMesh()
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
			{ "__tostring", ToString }, 
			{ "GetMesh", GetMesh },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<MeshProvider>(L, metalib, TypeID<Component>::name());
	}
};

class TextMesh_Wrapper {
	static int ToString(lua_State* L) {
		TextMesh& _p = *Lua::callerSharedPtr<TextMesh>(L, 0);
		lua_pushstring(L, String::Format("TextMesh@0x%p", _p.get()).c_str());
		return 1;
	}

	// void SetText(const std::string& value)
	static int SetText(lua_State* L) {
		TextMesh& _p = *Lua::callerSharedPtr<TextMesh>(L, 1);
		std::string value = Lua::get<std::string>(L, 2);
		_p->SetText(value);
		return 0;
	}

	// std::string GetText()
	static int GetText(lua_State* L) {
		TextMesh& _p = *Lua::callerSharedPtr<TextMesh>(L, 0);
		return Lua::push(L, _p->GetText());
	}

	// void SetFont(Font value)
	static int SetFont(lua_State* L) {
		TextMesh& _p = *Lua::callerSharedPtr<TextMesh>(L, 1);
		Font value = Lua::get<Font>(L, 2);
		_p->SetFont(value);
		return 0;
	}

	// Font GetFont()
	static int GetFont(lua_State* L) {
		TextMesh& _p = *Lua::callerSharedPtr<TextMesh>(L, 0);
		return Lua::push(L, _p->GetFont());
	}

	// void SetFontSize(uint value)
	static int SetFontSize(lua_State* L) {
		TextMesh& _p = *Lua::callerSharedPtr<TextMesh>(L, 1);
		uint value = Lua::get<uint>(L, 2);
		_p->SetFontSize(value);
		return 0;
	}

	// uint GetFontSize()
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
			{ "__tostring", ToString }, 
			{ "SetText", SetText },
			{ "GetText", GetText },
			{ "SetFont", SetFont },
			{ "GetFont", GetFont },
			{ "SetFontSize", SetFontSize },
			{ "GetFontSize", GetFontSize },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<TextMesh>(L, metalib, TypeID<MeshProvider>::name());
	}
};

class MeshFilter_Wrapper {
	static int ToString(lua_State* L) {
		MeshFilter& _p = *Lua::callerSharedPtr<MeshFilter>(L, 0);
		lua_pushstring(L, String::Format("MeshFilter@0x%p", _p.get()).c_str());
		return 1;
	}

	// void SetMesh(Mesh value)
	static int SetMesh(lua_State* L) {
		MeshFilter& _p = *Lua::callerSharedPtr<MeshFilter>(L, 1);
		Mesh value = Lua::get<Mesh>(L, 2);
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
			{ "__tostring", ToString }, 
			{ "SetMesh", SetMesh },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<MeshFilter>(L, metalib, TypeID<MeshProvider>::name());
	}
};
