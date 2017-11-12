#pragma once

#include <vector>
#include <gl/glew.h>

#include "surface.h"
#include "vertexarrayobject.h"
#include "internal/base/objectinternal.h"

class MeshInternal : public IMesh, public ObjectInternal {
	DEFINE_FACTORY_METHOD(Mesh)

public:
	MeshInternal();

	virtual void SetTopology(MeshTopology value) { meshTopology_ = value; }
	virtual MeshTopology GetTopology() { return meshTopology_; }

	virtual void SetTriangles(unsigned vertexCount, unsigned baseVertex, unsigned baseIndex);
	virtual void GetTriangles(unsigned& vertexCount, unsigned& baseVertex, unsigned& baseIndex);

private:
	MeshTopology meshTopology_;

	unsigned baseIndex_;
	unsigned baseVertex_;
	unsigned vertexCount_;
};

class SurfaceInternal : public ISurface, public ObjectInternal {
	DEFINE_FACTORY_METHOD(Surface)

public:
	SurfaceInternal();
	~SurfaceInternal();

public:
	virtual void SetAttribute(const SurfaceAttribute& value);
	virtual void AddMesh(Mesh mesh) { meshes_.push_back(mesh); }
	virtual int GetMeshCount() { return meshes_.size(); }
	virtual Mesh GetMesh(int index) { return meshes_[index]; }

	virtual void Bind();
	virtual void Unbind();

	virtual void UpdateInstanceBuffer(unsigned i, size_t size, void* data);

private:
	void Destroy();
	void UpdateGLBuffers(const SurfaceAttribute& attribute);
	size_t CalculateVBOCount(const SurfaceAttribute& attribute);

private:
	unsigned indexBuffer_;
	unsigned instanceBuffer_[2];

	VertexArrayObject vao_;
	std::vector<Mesh> meshes_;
};
