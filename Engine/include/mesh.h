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

SUEDE_DEFINE_OBJECT_POINTER(Mesh)

class SUEDE_API ISubMesh : public IObject {
	SUEDE_DEFINE_METATABLE_NAME(SubMesh)
	SUEDE_DECLARE_IMPLEMENTATION(SubMesh)

public:
	ISubMesh();

public:
	const TriangleBias& GetTriangleBias() const;
	void SetTriangleBias(const TriangleBias& value);
};

SUEDE_DEFINE_OBJECT_POINTER(SubMesh)

struct BlendAttribute {
	enum { Quality = 4, };

	uint indexes[Quality] = { 0 };
	float weights[Quality] = { 0 };
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

struct MeshAttribute {
	enum { TexCoordsCount = 4, };

	MeshTopology topology = MeshTopology::Triangles;

	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texCoords[TexCoordsCount];
	std::vector<glm::vec3> tangents;
	std::vector<BlendAttribute> blendAttrs;
	std::vector<uint> indexes;
	
	InstanceAttribute color;
	InstanceAttribute geometry;
};

class SUEDE_API IMesh : public IObject {
	SUEDE_DEFINE_METATABLE_NAME(Mesh)
	SUEDE_DECLARE_IMPLEMENTATION(Mesh)

public:
	IMesh();

public:
	typedef SuedeEnumerable<std::vector<SubMesh>::iterator> Enumerable;

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

	void AddSubMesh(SubMesh subMesh);
	uint GetSubMeshCount();
	SubMesh GetSubMesh(uint index);
	Enumerable GetSubMeshes();
	void RemoveSubMesh(uint index);

	MeshTopology GetTopology();
	uint GetNativePointer() const;

	const uint* MapIndexes();
	void UnmapIndexes();
	uint GetIndexCount();

	const glm::vec3* MapVertices();
	void UnmapVertices();
	uint GetVertexCount();

	void Bind();
	void Unbind();
	void ShareStorage(Mesh other);

	void UpdateInstanceBuffer(uint i, size_t size, void* data);
};

class SUEDE_API IMeshProvider : public IComponent {
	SUEDE_DECLARE_COMPONENT()
	SUEDE_DEFINE_METATABLE_NAME(MeshProvider)
	SUEDE_DECLARE_IMPLEMENTATION(MeshProvider)

public:
	Mesh GetMesh();

public:
	IMeshProvider(void * d);
};

SUEDE_DEFINE_OBJECT_POINTER(MeshProvider)

class SUEDE_API ITextMesh : public IMeshProvider {
	SUEDE_DECLARE_COMPONENT()
	SUEDE_DEFINE_METATABLE_NAME(TextMesh)
	SUEDE_DECLARE_IMPLEMENTATION(TextMesh)

public:
	ITextMesh();

public:
	void SetText(const std::string& value);
	std::string GetText();

	void SetFont(Font value);
	Font GetFont();
	
	void SetFontSize(uint value);
	uint GetFontSize();
};

SUEDE_DEFINE_OBJECT_POINTER(TextMesh)

class SUEDE_API IMeshFilter : public IMeshProvider {
	SUEDE_DECLARE_COMPONENT()
	SUEDE_DEFINE_METATABLE_NAME(MeshFilter)
	SUEDE_DECLARE_IMPLEMENTATION(MeshFilter)

public:
	IMeshFilter();

public:
	void SetMesh(Mesh value);
};

SUEDE_DEFINE_OBJECT_POINTER(MeshFilter)
