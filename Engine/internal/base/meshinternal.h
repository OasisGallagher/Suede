#pragma once

#include <vector>
#include <wrappers/gl.h>

#include "mesh.h"
#include "vertexarrayobject.h"
#include "internal/base/objectinternal.h"

enum VertexAttrib {
	VertexAttribPosition,
	VertexAttribTexCoord,
	VertexAttribNormal,
	VertexAttribTangent,
	VertexAttribBoneIndexes,
	VertexAttribBoneWeights,

	VertexAttribInstanceColor,
	VertexAttribInstanceGeometry,
};

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

class MeshInternal : virtual public IMesh, public ObjectInternal {
	DEFINE_FACTORY_METHOD(Mesh)

public:
	MeshInternal();
	MeshInternal(ObjectType type);
	~MeshInternal();

public:
	virtual void SetAttribute(const MeshAttribute& value);

	virtual void Bind();
	virtual void Unbind();

	virtual void AddSubMesh(SubMesh subMesh) { subMeshes_.push_back(subMesh); }
	virtual int GetSubMeshCount() { return subMeshes_.size(); }
	virtual SubMesh GetSubMesh(uint index) { return subMeshes_[index]; }
	virtual void RemoveSubMesh(uint index) { subMeshes_.erase(subMeshes_.begin() + index); }

	virtual MeshTopology GetTopology() { return attribute_.topology; }

	virtual const std::vector<uint>& GetIndexes() const { return attribute_.indexes; }
	virtual const std::vector<glm::vec3>& GetVertices() const { return attribute_.positions; }

	virtual void UpdateInstanceBuffer(uint i, size_t size, void* data);

private:
	void Destroy();
	void UpdateGLBuffers(const MeshAttribute& attribute);
	int CalculateVBOCount(const MeshAttribute& attribute);

private:
	uint indexBuffer_;
	MeshAttribute attribute_;

	VertexArrayObject vao_;

	uint instanceBuffer_[2];
	std::vector<SubMesh> subMeshes_;
};

class TextMeshInternal : public ITextMesh, public MeshInternal {
	DEFINE_FACTORY_METHOD(TextMesh)

public:
	TextMeshInternal();

public:
	virtual void SetText(const std::string& value);
	virtual std::string GetText() { return text_; }

	virtual void SetFont(Font value);
	virtual Font GetFont() { return font_; }

	virtual void SetFontSize(uint value);
	virtual uint GetFontSize() { return size_; }

private:
	void RebuildMesh();
	void InitializeMeshAttribute(MeshAttribute& attribute, const std::wstring& wtext);

private:
	uint size_;
	Font font_;

	std::string text_;
};
