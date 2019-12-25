// Warning: this file is generated by wrapper.py.

#pragma once

#include "mesh.h"

#include "lua++.h"
#include "tools/string.h"

class TriangleBias_Wrapper {
	static int NewTriangleBias(lua_State* L) {
		return Lua::newObject<TriangleBias>(L);
	}

	static int ToString(lua_State* L) {
		TriangleBias* _p = Lua::callerPtr<TriangleBias>(L);

		lua_pushstring(L, String::Format("TriangleBias@0x%p", _p).c_str());
		return 1;
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<TriangleBias>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		funcs.push_back(luaL_Reg { "NewTriangleBias", NewTriangleBias });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deletePtr<TriangleBias> },
			{ "__tostring", ToString }, 
			{ nullptr, nullptr }
		};

		Lua::initMetatable<TriangleBias>(L, metalib, nullptr);
	}
};

class SubMesh_Wrapper {
	static int NewSubMesh(lua_State* L) {
		return Lua::newObject<SubMesh>(L);
	}

	static int ToString(lua_State* L) {
		SubMesh* _p = Lua::callerPtr<SubMesh>(L);

		lua_pushstring(L, String::Format("SubMesh@0x%p", _p).c_str());
		return 1;
	}

	// void SetTriangleBias(const TriangleBias& value)
	static int SetTriangleBias(lua_State* L) {
		SubMesh* _p = Lua::callerPtr<SubMesh>(L);
		TriangleBias value = Lua::get<TriangleBias>(L, 2);
		
		_p->SetTriangleBias(value);
		return 0;
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<SubMesh>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		funcs.push_back(luaL_Reg { "NewSubMesh", NewSubMesh });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deletePtr<SubMesh> },
			{ "__tostring", ToString }, 
			{ "SetTriangleBias", SetTriangleBias },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<SubMesh>(L, metalib, TypeID<Object>::string());
	}
};

class BlendAttribute_Wrapper {
	static int NewBlendAttribute(lua_State* L) {
		return Lua::newObject<BlendAttribute>(L);
	}

	static int ToString(lua_State* L) {
		BlendAttribute* _p = Lua::callerPtr<BlendAttribute>(L);

		lua_pushstring(L, String::Format("BlendAttribute@0x%p", _p).c_str());
		return 1;
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<BlendAttribute>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		funcs.push_back(luaL_Reg { "NewBlendAttribute", NewBlendAttribute });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deletePtr<BlendAttribute> },
			{ "__tostring", ToString }, 
			{ nullptr, nullptr }
		};

		Lua::initMetatable<BlendAttribute>(L, metalib, nullptr);
	}
};

class InstanceAttribute_Wrapper {
	static int NewInstanceAttribute(lua_State* L) {
		return Lua::newObject<InstanceAttribute>(L);
	}

	static int ToString(lua_State* L) {
		InstanceAttribute* _p = Lua::callerPtr<InstanceAttribute>(L);

		lua_pushstring(L, String::Format("InstanceAttribute@0x%p", _p).c_str());
		return 1;
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<InstanceAttribute>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		funcs.push_back(luaL_Reg { "NewInstanceAttribute", NewInstanceAttribute });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deletePtr<InstanceAttribute> },
			{ "__tostring", ToString }, 
			{ nullptr, nullptr }
		};

		Lua::initMetatable<InstanceAttribute>(L, metalib, nullptr);
	}
};

class MeshAttribute_Wrapper {
	static int NewMeshAttribute(lua_State* L) {
		return Lua::newObject<MeshAttribute>(L);
	}

	static int ToString(lua_State* L) {
		MeshAttribute* _p = Lua::callerPtr<MeshAttribute>(L);

		lua_pushstring(L, String::Format("MeshAttribute@0x%p", _p).c_str());
		return 1;
	}

	static int ToStringStatic(lua_State* L) {
		lua_pushstring(L, "static MeshAttribute");
		return 1;
	}

	static int MeshAttributeStatic(lua_State* L) {
		lua_newtable(L);

		luaL_Reg funcs[] = {
			{ "GetPrimitiveAttribute", GetPrimitiveAttribute },
			{"__tostring", ToStringStatic },
			{ nullptr, nullptr }
		};

		luaL_setfuncs(L, funcs, 0);

		return 1;
	}
	// static void GetPrimitiveAttribute(PrimitiveType type, MeshAttribute& attribute, float scale)
	static int GetPrimitiveAttribute(lua_State* L) {
		float scale = Lua::get<float>(L, 3);
		MeshAttribute attribute = Lua::get<MeshAttribute>(L, 2);
		PrimitiveType type = Lua::get<PrimitiveType>(L, 1);
		
		MeshAttribute::GetPrimitiveAttribute(type, attribute, scale);
		return 0;
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<MeshAttribute>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		funcs.push_back(luaL_Reg { "NewMeshAttribute", NewMeshAttribute });

		fields.push_back(luaL_Reg{ "MeshAttribute", MeshAttributeStatic });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deletePtr<MeshAttribute> },
			{ "__tostring", ToString }, 
			{ nullptr, nullptr }
		};

		Lua::initMetatable<MeshAttribute>(L, metalib, nullptr);
	}
};

class Mesh_Wrapper {
	static int NewMesh(lua_State* L) {
		return Lua::newObject<Mesh>(L);
	}

	static int ToString(lua_State* L) {
		Mesh* _p = Lua::callerPtr<Mesh>(L);

		lua_pushstring(L, String::Format("Mesh@0x%p", _p).c_str());
		return 1;
	}

	static int ToStringStatic(lua_State* L) {
		lua_pushstring(L, "static Mesh");
		return 1;
	}

	static int MeshStatic(lua_State* L) {
		lua_newtable(L);

		luaL_Reg funcs[] = {
			{ "FromAttribute", FromAttribute },
			{ "CreatePrimitive", CreatePrimitive },
			{ "CreateInstancedPrimitive", CreateInstancedPrimitive },
			{"__tostring", ToStringStatic },
			{ nullptr, nullptr }
		};

		luaL_setfuncs(L, funcs, 0);

		return 1;
	}
	// void CreateStorage()
	static int CreateStorage(lua_State* L) {
		Mesh* _p = Lua::callerPtr<Mesh>(L);
		_p->CreateStorage();
		return 0;
	}

	// void SetAttribute(const MeshAttribute& value)
	static int SetAttribute(lua_State* L) {
		Mesh* _p = Lua::callerPtr<Mesh>(L);
		MeshAttribute value = Lua::get<MeshAttribute>(L, 2);
		
		_p->SetAttribute(value);
		return 0;
	}

	// uint GetSubMeshCount()
	static int GetSubMeshCount(lua_State* L) {
		Mesh* _p = Lua::callerPtr<Mesh>(L);
		return Lua::push(L, _p->GetSubMeshCount());
	}

	// void RemoveSubMesh(uint index)
	static int RemoveSubMesh(lua_State* L) {
		Mesh* _p = Lua::callerPtr<Mesh>(L);
		uint index = Lua::get<uint>(L, 2);
		
		_p->RemoveSubMesh(index);
		return 0;
	}

	// MeshTopology GetTopology()
	static int GetTopology(lua_State* L) {
		Mesh* _p = Lua::callerPtr<Mesh>(L);
		return Lua::push(L, _p->GetTopology());
	}

	// uint GetNativePointer()
	static int GetNativePointer(lua_State* L) {
		Mesh* _p = Lua::callerPtr<Mesh>(L);
		return Lua::push(L, _p->GetNativePointer());
	}

	// void UnmapIndexes()
	static int UnmapIndexes(lua_State* L) {
		Mesh* _p = Lua::callerPtr<Mesh>(L);
		_p->UnmapIndexes();
		return 0;
	}

	// uint GetIndexCount()
	static int GetIndexCount(lua_State* L) {
		Mesh* _p = Lua::callerPtr<Mesh>(L);
		return Lua::push(L, _p->GetIndexCount());
	}

	// void UnmapVertices()
	static int UnmapVertices(lua_State* L) {
		Mesh* _p = Lua::callerPtr<Mesh>(L);
		_p->UnmapVertices();
		return 0;
	}

	// uint GetVertexCount()
	static int GetVertexCount(lua_State* L) {
		Mesh* _p = Lua::callerPtr<Mesh>(L);
		return Lua::push(L, _p->GetVertexCount());
	}

	// void Bind()
	static int Bind(lua_State* L) {
		Mesh* _p = Lua::callerPtr<Mesh>(L);
		_p->Bind();
		return 0;
	}

	// void Unbind()
	static int Unbind(lua_State* L) {
		Mesh* _p = Lua::callerPtr<Mesh>(L);
		_p->Unbind();
		return 0;
	}

	// static ref_ptr<Mesh> FromAttribute(const MeshAttribute& attribute)
	static int FromAttribute(lua_State* L) {
		MeshAttribute attribute = Lua::get<MeshAttribute>(L, 1);
		
		return Lua::push(L, Mesh::FromAttribute(attribute));
	}

	// static ref_ptr<Mesh> CreatePrimitive(PrimitiveType type, float scale)
	static int CreatePrimitive(lua_State* L) {
		float scale = Lua::get<float>(L, 2);
		PrimitiveType type = Lua::get<PrimitiveType>(L, 1);
		
		return Lua::push(L, Mesh::CreatePrimitive(type, scale));
	}

	// static ref_ptr<Mesh> CreateInstancedPrimitive(PrimitiveType type, float scale, const InstanceAttribute& color, const InstanceAttribute& geometry)
	static int CreateInstancedPrimitive(lua_State* L) {
		InstanceAttribute geometry = Lua::get<InstanceAttribute>(L, 4);
		InstanceAttribute color = Lua::get<InstanceAttribute>(L, 3);
		float scale = Lua::get<float>(L, 2);
		PrimitiveType type = Lua::get<PrimitiveType>(L, 1);
		
		return Lua::push(L, Mesh::CreateInstancedPrimitive(type, scale, color, geometry));
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<Mesh>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		funcs.push_back(luaL_Reg { "NewMesh", NewMesh });

		fields.push_back(luaL_Reg{ "Mesh", MeshStatic });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deletePtr<Mesh> },
			{ "__tostring", ToString }, 
			{ "CreateStorage", CreateStorage },
			{ "SetAttribute", SetAttribute },
			{ "GetSubMeshCount", GetSubMeshCount },
			{ "RemoveSubMesh", RemoveSubMesh },
			{ "GetTopology", GetTopology },
			{ "GetNativePointer", GetNativePointer },
			{ "UnmapIndexes", UnmapIndexes },
			{ "GetIndexCount", GetIndexCount },
			{ "UnmapVertices", UnmapVertices },
			{ "GetVertexCount", GetVertexCount },
			{ "Bind", Bind },
			{ "Unbind", Unbind },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<Mesh>(L, metalib, TypeID<Object>::string());
	}
};

class MeshProvider_Wrapper {
	static int ToString(lua_State* L) {
		MeshProvider* _p = Lua::callerPtr<MeshProvider>(L);

		lua_pushstring(L, String::Format("MeshProvider@0x%p", _p).c_str());
		return 1;
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<MeshProvider>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		luaL_Reg metalib[] = {
			{ "__gc", Lua::deletePtr<MeshProvider> },
			{ "__tostring", ToString }, 
			{ nullptr, nullptr }
		};

		Lua::initMetatable<MeshProvider>(L, metalib, TypeID<Component>::string());
	}
};

class TextMesh_Wrapper {
	static int NewTextMesh(lua_State* L) {
		return Lua::newObject<TextMesh>(L);
	}

	static int ToString(lua_State* L) {
		TextMesh* _p = Lua::callerPtr<TextMesh>(L);

		lua_pushstring(L, String::Format("TextMesh@0x%p", _p).c_str());
		return 1;
	}

	// void SetText(const std::string& value)
	static int SetText(lua_State* L) {
		TextMesh* _p = Lua::callerPtr<TextMesh>(L);
		std::string value = Lua::get<std::string>(L, 2);
		
		_p->SetText(value);
		return 0;
	}

	// std::string GetText()
	static int GetText(lua_State* L) {
		TextMesh* _p = Lua::callerPtr<TextMesh>(L);
		return Lua::push(L, _p->GetText());
	}

	// void SetFontSize(uint value)
	static int SetFontSize(lua_State* L) {
		TextMesh* _p = Lua::callerPtr<TextMesh>(L);
		uint value = Lua::get<uint>(L, 2);
		
		_p->SetFontSize(value);
		return 0;
	}

	// uint GetFontSize()
	static int GetFontSize(lua_State* L) {
		TextMesh* _p = Lua::callerPtr<TextMesh>(L);
		return Lua::push(L, _p->GetFontSize());
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<TextMesh>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		funcs.push_back(luaL_Reg { "NewTextMesh", NewTextMesh });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deletePtr<TextMesh> },
			{ "__tostring", ToString }, 
			{ "SetText", SetText },
			{ "GetText", GetText },
			{ "SetFontSize", SetFontSize },
			{ "GetFontSize", GetFontSize },
			{ nullptr, nullptr }
		};

		Lua::initMetatable<TextMesh>(L, metalib, TypeID<MeshProvider>::string());
	}
};

class MeshFilter_Wrapper {
	static int NewMeshFilter(lua_State* L) {
		return Lua::newObject<MeshFilter>(L);
	}

	static int ToString(lua_State* L) {
		MeshFilter* _p = Lua::callerPtr<MeshFilter>(L);

		lua_pushstring(L, String::Format("MeshFilter@0x%p", _p).c_str());
		return 1;
	}

public:
	static void create(lua_State* L) {
		Lua::createMetatable<MeshFilter>(L);
	}
	
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {
		funcs.push_back(luaL_Reg { "NewMeshFilter", NewMeshFilter });

		luaL_Reg metalib[] = {
			{ "__gc", Lua::deletePtr<MeshFilter> },
			{ "__tostring", ToString }, 
			{ nullptr, nullptr }
		};

		Lua::initMetatable<MeshFilter>(L, metalib, TypeID<MeshProvider>::string());
	}
};
