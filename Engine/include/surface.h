#pragma once
#include <vector>

#include "object.h"
#include "material.h"

enum MeshTopology {
	MeshTopologyTriangles,
	MeshTopologyTriangleStripes,
};

class IMesh : virtual public IObject {
public:
	virtual void SetTopology(MeshTopology value) = 0;
	virtual MeshTopology GetTopology() = 0;

	virtual void SetTriangles(uint vertexCount, uint baseVertex, uint baseIndex) = 0;
	virtual void GetTriangles(uint& vertexCount, uint& baseVertex, uint& baseIndex) = 0;
};

SUEDE_DEFINE_OBJECT_POINTER(Mesh);

struct BlendAttribute {
	enum {
		Quality = 4,
	};

	uint indexes[Quality];
	float weights[Quality];
};

struct InstanceAttribute {
	InstanceAttribute(uint count = 0, int divisor = 0) 
		: count(count), divisor(divisor) {}

	int divisor;
	uint count;
};

struct SurfaceAttribute {
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texCoords;
	std::vector<glm::vec3> tangents;
	std::vector<BlendAttribute> blendAttrs;
	std::vector<uint> indexes;
	
	InstanceAttribute color;
	InstanceAttribute geometry;
};

class SUEDE_API ISurface : virtual public IObject {
public:
	virtual void SetAttribute(const SurfaceAttribute& value) = 0;
	virtual void AddMesh(Mesh mesh) = 0;
	virtual int GetMeshCount() = 0;
	virtual Mesh GetMesh(int index) = 0;

	virtual void UpdateInstanceBuffer(uint i, size_t size, void* data) = 0;

	virtual void Bind() = 0;
	virtual void Unbind() = 0;
};

SUEDE_DEFINE_OBJECT_POINTER(Surface);
