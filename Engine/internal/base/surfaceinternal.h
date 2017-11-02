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

	virtual void SetMaterialTextures(const MaterialTextures& value) { textures_ = value; }
	virtual MaterialTextures& GetMaterialTextures() { return textures_; }

	virtual void SetPrimaryType(PrimaryType value) { primaryType_ = value; }
	virtual PrimaryType GetPrimaryType() { return primaryType_; }

	virtual void SetTriangles(unsigned vertexCount, unsigned baseVertex, unsigned baseIndex);
	virtual void GetTriangles(unsigned& vertexCount, unsigned& baseVertex, unsigned& baseIndex);

private:
	PrimaryType primaryType_;
	MaterialTextures textures_;

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

	virtual void UpdateUserBuffer(unsigned i, size_t size, void* data);

private:
	// TODO: dynamic vbo count...
	enum {
		VBOPositions,
		VBOTexCoords,
		VBONormals,
		VBOTangents,
		VBOBones,
		VBOIndexes,
		VBOUser0,
		VBOUser1,
		VBOUser2,
		VBOCount,
	};

	void Destroy();
	void UpdateGLBuffers(const SurfaceAttribute& attribute);

private:
	VertexArrayObject vao_;
	std::vector<Mesh> meshes_;
};
