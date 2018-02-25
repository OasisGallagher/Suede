#pragma once
#include <vector>

#include "font.h"
#include "object.h"
#include "material.h"

struct TriangleBase {
	uint indexCount;
	uint baseIndex;
	uint baseVertex;
}; 

class ISubMesh : virtual public IObject {
public:
	virtual const TriangleBase& GetTriangles() const = 0;
	virtual void SetTriangles(const TriangleBase& value) = 0;
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
	InstanceAttribute(uint c = 0, int d = 0) 
		: count(c), divisor(d) {
	}

	uint count;
	int divisor;
};

enum MeshTopology {
	MeshTopologyTriangles,
	MeshTopologyTriangleStripes,
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

SUEDE_DEFINE_OBJECT_POINTER(Mesh);

class SUEDE_API IMesh : virtual public IObject {
public:
	virtual void SetAttribute(const MeshAttribute& value) = 0;

	virtual void AddSubMesh(SubMesh subMesh) = 0;
	virtual int GetSubMeshCount() = 0;
	virtual SubMesh GetSubMesh(uint index) = 0;
	virtual void RemoveSubMesh(uint index) = 0;

	virtual MeshTopology GetTopology() = 0;

	virtual bool MapIndexes(uint** data, uint* count) = 0;
	virtual void UnmapIndexes() = 0;

	virtual bool MapVertices(glm::vec3** data, uint* count) = 0;
	virtual void UnmapVertices() = 0;

	virtual void Bind() = 0;
	virtual void Unbind() = 0;
	virtual void MakeShared(Mesh other) = 0;

	virtual void UpdateInstanceBuffer(uint i, size_t size, void* data) = 0;
};

SUEDE_DECLARE_OBJECT_CREATER(Mesh);

class SUEDE_API ITextMesh : virtual public IMesh {
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
