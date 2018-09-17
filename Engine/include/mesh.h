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

SUEDE_DEFINE_OBJECT_POINTER(Mesh);

class ISubMesh : virtual public IObject {
public:
	virtual const TriangleBias& GetTriangleBias() const = 0;
	virtual void SetTriangleBias(const TriangleBias& value) = 0;
};

SUEDE_DEFINE_OBJECT_POINTER(SubMesh);
SUEDE_DECLARE_OBJECT_CREATER(SubMesh);

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
	InstanceAttribute(uint c = 0, uint d = 0) 
		: count(c), divisor(d) {
	}

	uint count;
	uint divisor;
};

enum class MeshTopology {
	Points,

	Lines,
	LineStripe,

	Triangles,
	TriangleStripe,
};

struct MeshAttribute {
	MeshTopology topology;

	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texCoords;
	std::vector<glm::vec3> tangents;
	std::vector<BlendAttribute> blendAttrs;
	std::vector<uint> indexes;
	
	InstanceAttribute color;
	InstanceAttribute geometry;
};

class SUEDE_API IMesh : virtual public IObject {
public:
	typedef SuedeEnumerable<std::vector<SubMesh>::iterator> Enumerable;

public:
	/**
	 * @brief: create an empty storage for sharing.
	 */
	virtual void CreateStorage() = 0;
	virtual void SetAttribute(const MeshAttribute& value) = 0;

	/**
	 * @returns bounds measured in the world space.
	 */
	virtual const Bounds& GetBounds() const = 0;
	virtual void SetBounds(const Bounds& value) = 0;

	virtual void AddSubMesh(SubMesh subMesh) = 0;
	virtual int GetSubMeshCount() = 0;
	virtual SubMesh GetSubMesh(uint index) = 0;
	virtual Enumerable GetSubMeshes() = 0;
	virtual void RemoveSubMesh(uint index) = 0;

	virtual MeshTopology GetTopology() = 0;
	virtual uint GetNativePointer() const = 0;

	virtual uint* MapIndexes() = 0;
	virtual void UnmapIndexes() = 0;
	virtual uint GetIndexCount() = 0;

	virtual glm::vec3* MapVertices() = 0;
	virtual void UnmapVertices() = 0;
	virtual uint GetVertexCount() = 0;

	virtual void Bind() = 0;
	virtual void Unbind() = 0;
	virtual void ShareStorage(Mesh other) = 0;

	virtual void UpdateInstanceBuffer(uint i, size_t size, void* data) = 0;
};

SUEDE_DECLARE_OBJECT_CREATER(Mesh);

class SUEDE_API ITextMesh : virtual public IComponent {
	SUEDE_DECLARE_COMPONENT()

public:
	virtual void SetText(const std::string& value) = 0;
	virtual std::string GetText() = 0;

	virtual void SetFont(Font value) = 0;
	virtual Font GetFont() = 0;
	
	virtual void SetFontSize(uint value) = 0;
	virtual uint GetFontSize() = 0;
};

SUEDE_DEFINE_OBJECT_POINTER(TextMesh);
SUEDE_DECLARE_OBJECT_CREATER(TextMesh);

class SUEDE_API IMeshFilter : virtual public IComponent {
	SUEDE_DECLARE_COMPONENT()

public:
	virtual void SetMesh(Mesh value) = 0;
	virtual Mesh GetMesh() = 0;
};

SUEDE_DEFINE_OBJECT_POINTER(MeshFilter);
SUEDE_DECLARE_OBJECT_CREATER(MeshFilter);
