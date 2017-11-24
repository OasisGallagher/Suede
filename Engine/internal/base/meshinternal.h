#pragma once

#include <vector>
#include <gl/glew.h>

#include "mesh.h"
#include "vertexarrayobject.h"
#include "internal/base/objectinternal.h"

class SubMeshInternal : public ISubMesh, public ObjectInternal {
	DEFINE_FACTORY_METHOD(SubMesh)

public:
	SubMeshInternal();

	virtual void SetTriangles(uint indexCount, uint baseVertex, uint baseIndex);
	virtual void GetTriangles(uint& indexCount, uint& baseVertex, uint& baseIndex);

private:
	uint baseIndex_;
	uint baseVertex_;
	uint indexCount_;
};

class MeshInternal : public IMesh, public ObjectInternal {
	DEFINE_FACTORY_METHOD(Mesh)

public:
	MeshInternal();
	~MeshInternal();

public:
	virtual void SetAttribute(const MeshAttribute& value);
	virtual void AddSubMesh(SubMesh subMesh) { subMeshes_.push_back(subMesh); }
	virtual int GetSubMeshCount() { return subMeshes_.size(); }
	virtual SubMesh GetSubMesh(int index) { return subMeshes_[index]; }

	virtual void SetTopology(MeshTopology value) { meshTopology_ = value; }
	virtual MeshTopology GetTopology() { return meshTopology_; }

	virtual void Bind();
	virtual void Unbind();

	virtual void UpdateInstanceBuffer(uint i, size_t size, void* data);

private:
	void Destroy();
	void UpdateGLBuffers(const MeshAttribute& attribute);
	int CalculateVBOCount(const MeshAttribute& attribute);

private:
	uint indexBuffer_;
	uint instanceBuffer_[2];

	MeshTopology meshTopology_;

	VertexArrayObject vao_;
	std::vector<SubMesh> subMeshes_;
};
