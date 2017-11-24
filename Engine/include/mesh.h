#pragma once
#include <vector>

#include "object.h"
#include "material.h"

class ISubMesh : virtual public IObject {
public:
	virtual void SetTriangles(uint indexCount, uint baseVertex, uint baseIndex) = 0;
	virtual void GetTriangles(uint& indexCount, uint& baseVertex, uint& baseIndex) = 0;
};

SUEDE_DEFINE_OBJECT_POINTER(SubMesh);

struct BlendAttribute {
	enum {
		Quality = 4,
	};

	uint indexes[Quality];
	float weights[Quality];
};

struct InstanceAttribute {
	uint count;
	int divisor;
};

struct MeshAttribute {
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texCoords;
	std::vector<glm::vec3> tangents;
	std::vector<BlendAttribute> blendAttrs;
	std::vector<uint> indexes;
	
	InstanceAttribute color;
	InstanceAttribute geometry;
};

enum MeshTopology {
	MeshTopologyTriangles,
	MeshTopologyTriangleStripes,
};

class SUEDE_API IMesh : virtual public IObject {
public:
	virtual void SetAttribute(const MeshAttribute& value) = 0;
	virtual void AddSubMesh(SubMesh subMesh) = 0;
	virtual int GetSubMeshCount() = 0;
	virtual SubMesh GetSubMesh(int index) = 0;

	virtual void SetTopology(MeshTopology value) = 0;
	virtual MeshTopology GetTopology() = 0;

	virtual void UpdateInstanceBuffer(uint i, size_t size, void* data) = 0;

	virtual void Bind() = 0;
	virtual void Unbind() = 0;
};

SUEDE_DEFINE_OBJECT_POINTER(Mesh);

class SUEDE_API ITextMesh : virtual public IObject {

};

SUEDE_DEFINE_OBJECT_POINTER(TextMesh);
