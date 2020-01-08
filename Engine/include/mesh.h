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

class SUEDE_API Geometry : public Object {
	SUEDE_DECLARE_IMPLEMENTATION(Geometry)

public:
	enum {
		TexCoordsCount = 4,
	};

	Geometry();

public:
	static Geometry* GetPrimitive(PrimitiveType type);
	static ref_ptr<Geometry> CreatePrimitive(PrimitiveType type, float scale);

public:
	void SetVertices(const Vector3* values, int count);
	const Vector3* GetVertices() const;
	uint GetVertexCount() const;

	void SetNormals(const Vector3* values, int count);
	const Vector3* GetNormals() const;
	uint GetNormalCount() const;

	void SetTangents(const Vector3* values, int count);
	const Vector3* GetTangents() const;
	uint GetTangentCount() const;

	void SetTexCoords(int index, const Vector2* values, int count);
	const Vector2* GetTexCoords(int index) const;
	uint GetTexCoordCount(int index) const;

	void SetBlendAttributes(const BlendAttribute* values, int count);
	const BlendAttribute* GetBlendAttributes() const;
	uint GetBlendAttributeCount() const;

	void SetIndexes(const uint* values, int count);
	const uint* GetIndexes() const;
	uint GetIndexCount() const;

	void SetColorInstanceAttribute(const InstanceAttribute& value);
	void SetGeometryInstanceAttribute(const InstanceAttribute& value);

	void UpdateInstanceBuffer(int index, size_t size, void* data);

	void SetTopology(MeshTopology value);
	MeshTopology GetTopology() const;
};

class SUEDE_API Mesh : public Object {
	SUEDE_DEFINE_METATABLE_NAME(Mesh)
	SUEDE_DECLARE_IMPLEMENTATION(Mesh)

public:
	Mesh();

public:
	static ref_ptr<Mesh> FromGeometry(Geometry* geometry);

public:
	void SetGeometry(Geometry* value);
	Geometry* GetGeometry();

	void AddSubMesh(SubMesh* subMesh);
	uint GetSubMeshCount();
	SubMesh* GetSubMesh(uint index);
	void RemoveSubMesh(uint index);
	
	void Bind();
	void Unbind();

	const Bounds& GetBounds();
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
