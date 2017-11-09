#pragma once
#include <vector>
#include <gl/glew.h>

#include "object.h"
#include "material.h"

enum MeshTopology {
	MeshTopologyTriangles,
	MeshTopologyTriangleStripes,
};

struct MaterialTextures {
	Texture bump;
	Texture albedo;
	Texture specular;
};

class IMesh : virtual public IObject {
public:
	virtual void SetTopology(MeshTopology value) = 0;
	virtual MeshTopology GetTopology() = 0;

	virtual void SetMaterialTextures(const MaterialTextures& value) = 0;
	virtual MaterialTextures& GetMaterialTextures() = 0;

	virtual void SetTriangles(unsigned vertexCount, unsigned baseVertex, unsigned baseIndex) = 0;
	virtual void GetTriangles(unsigned& vertexCount, unsigned& baseVertex, unsigned& baseIndex) = 0;
};

typedef std::shared_ptr<IMesh> Mesh;

struct BlendAttribute {
	enum {
		Quality = 4,
	};

	unsigned indexes[Quality];
	float weights[Quality];
};

struct UserAttribute {
	int divisor;
	std::vector<glm::vec4> data;
};

struct SurfaceAttribute {
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texCoords;
	std::vector<glm::vec3> tangents;
	std::vector<BlendAttribute> blendAttrs;
	std::vector<unsigned> indexes;

	UserAttribute user0;
	UserAttribute user1;
	UserAttribute user2;
};

class ENGINE_EXPORT ISurface : virtual public IObject {
public:
	virtual void SetAttribute(const SurfaceAttribute& value) = 0;
	virtual void AddMesh(Mesh mesh) = 0;
	virtual int GetMeshCount() = 0;
	virtual Mesh GetMesh(int index) = 0;

	virtual void UpdateUserBuffer(unsigned i, size_t size, void* data) = 0;

	virtual void Bind() = 0;
	virtual void Unbind() = 0;
};

typedef std::shared_ptr<ISurface> Surface;
