#pragma once
#include <vector>

#include "font.h"
#include "bounds.h"
#include "material.h"
#include "component.h"
#include "tools/enum.h"

struct TriangleBias {
	uint indexCount;
	uint baseIndex;
	uint baseVertex;
};

class SubMesh : public Object {
	SUEDE_DEFINE_METATABLE_NAME(SubMesh)
	SUEDE_DECLARE_IMPLEMENTATION(SubMesh)

public:
	SubMesh();

public:
	const TriangleBias& GetTriangleBias() const;
	void SetTriangleBias(const TriangleBias& value);
};

struct BlendAttribute {
	enum {
		Quality = 4,
	};

	BlendAttribute() {
		memset(this, 0, sizeof(BlendAttribute));
	}

	uint indexes[Quality];
	float weights[Quality];
};

struct InstanceAttribute {
	uint count = 0;
	uint divisor = 0;
};

enum class MeshTopology {
	Points,

	Lines,
	LineStripe,

	Triangles,
	TriangleStripe,
	TriangleFan,
};

BETTER_ENUM(PrimitiveType, int,
	Quad,
	Cube
)

struct MeshAttribute {
	enum {
		TexCoordsCount = 4,
	};

	MeshTopology topology;

	std::vector<Vector3> positions;
	std::vector<Vector3> normals;
	std::vector<Vector2> texCoords[TexCoordsCount];
	std::vector<Vector3> tangents;
	std::vector<BlendAttribute> blendAttrs;
	std::vector<uint> indexes;
	
	InstanceAttribute color;
	InstanceAttribute geometry;

	static void GetPrimitiveAttribute(PrimitiveType type, MeshAttribute& attribute, float scale);
};

class SUEDE_API Mesh : public Object {
	SUEDE_DEFINE_METATABLE_NAME(Mesh)
	SUEDE_DECLARE_IMPLEMENTATION(Mesh)

public:
	Mesh();

public:
	/**
	 * @brief: create an empty storage for sharing.
	 */
	void CreateStorage();
	void SetAttribute(const MeshAttribute& value);

	/**
	 * @returns bounds measured in the world space.
	 */
	//const Bounds& GetBounds() const;
	//void SetBounds(const Bounds& value);

	void AddSubMesh(SubMesh* subMesh);
	uint GetSubMeshCount();
	SubMesh* GetSubMesh(uint index);
	void RemoveSubMesh(uint index);

	MeshTopology GetTopology();
	uint GetNativePointer() const;

	const uint* MapIndexes();
	void UnmapIndexes();
	uint GetIndexCount();

	const Vector3* MapVertices();
	void UnmapVertices();
	uint GetVertexCount();

	void Bind();
	void Unbind();
	void ShareStorage(Mesh* other);

	void UpdateInstanceBuffer(uint i, size_t size, void* data);

	static Mesh* GetPrimitive(PrimitiveType type);
	static ref_ptr<Mesh> FromAttribute(const MeshAttribute& attribute);
	static ref_ptr<Mesh> CreatePrimitive(PrimitiveType type, float scale);
	static ref_ptr<Mesh> CreateInstancedPrimitive(PrimitiveType type, float scale, const InstanceAttribute& color, const InstanceAttribute& geometry);
};

class SUEDE_API MeshProvider : public Component {
	SUEDE_DECLARE_COMPONENT()
	SUEDE_DEFINE_METATABLE_NAME(MeshProvider)
	SUEDE_DECLARE_IMPLEMENTATION(MeshProvider)

public:
	Mesh* GetMesh();

public:
	MeshProvider(void * d);
};

class SUEDE_API TextMesh : public MeshProvider {
	SUEDE_DECLARE_COMPONENT()
	SUEDE_DEFINE_METATABLE_NAME(TextMesh)
	SUEDE_DECLARE_IMPLEMENTATION(TextMesh)

public:
	TextMesh();

public:
	void SetText(const std::string& value);
	std::string GetText();

	void SetFont(Font* value);
	Font* GetFont();
	
	void SetFontSize(uint value);
	uint GetFontSize();
};

class SUEDE_API MeshFilter : public MeshProvider {
	SUEDE_DECLARE_COMPONENT()
	SUEDE_DEFINE_METATABLE_NAME(MeshFilter)
	SUEDE_DECLARE_IMPLEMENTATION(MeshFilter)

public:
	MeshFilter();

public:
	void SetMesh(Mesh* value);
};
