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

	virtual void SetTriangles(unsigned vertexCount, unsigned baseVertex, unsigned baseIndex) = 0;
	virtual void GetTriangles(unsigned& vertexCount, unsigned& baseVertex, unsigned& baseIndex) = 0;
};

SUEDE_DEFINE_OBJECT_POINTER(Mesh);

struct BlendAttribute {
	enum {
		Quality = 4,
	};

	unsigned indexes[Quality];
	float weights[Quality];
};

struct InstanceAttribute {
	InstanceAttribute(unsigned count = 0, int divisor = 0) 
		: count(count), divisor(divisor) {}

	int divisor;
	unsigned count;
};

struct SurfaceAttribute {
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texCoords;
	std::vector<glm::vec3> tangents;
	std::vector<BlendAttribute> blendAttrs;
	std::vector<unsigned> indexes;
	
	InstanceAttribute color;
	InstanceAttribute geometry;
};

class SUEDE_API ISurface : virtual public IObject {
public:
	virtual void SetAttribute(const SurfaceAttribute& value) = 0;
	virtual void AddMesh(Mesh mesh) = 0;
	virtual int GetMeshCount() = 0;
	virtual Mesh GetMesh(int index) = 0;

	virtual void UpdateInstanceBuffer(unsigned i, size_t size, void* data) = 0;

	virtual void Bind() = 0;
	virtual void Unbind() = 0;
};

SUEDE_DEFINE_OBJECT_POINTER(Surface);
