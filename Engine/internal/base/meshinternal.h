#pragma once

#include <vector>
#include <wrappers/gl.h>

#include "vao.h"
#include "mesh.h"
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

	virtual void SetMesh(Mesh value) { mesh_ = value; }
	virtual Mesh GetMesh() { return mesh_.lock(); }

	virtual void SetTriangles(const TriangleBase& value) { base_ = value; }
	virtual const TriangleBase& GetTriangles() const { return base_; }

private:
	TriangleBase base_;
	std::weak_ptr<Mesh::element_type> mesh_;
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
	virtual void ShareStorage(Mesh other);

	virtual void AddSubMesh(SubMesh subMesh);
	virtual int GetSubMeshCount() { return subMeshes_.size(); }
	virtual SubMesh GetSubMesh(uint index) { return subMeshes_[index]; }
	virtual void RemoveSubMesh(uint index);

	virtual MeshTopology GetTopology() { return topology_; }
	virtual uint GetNativePointer() const { return vao_->GetNativePointer(); }

	virtual uint* MapIndexes();
	virtual void UnmapIndexes();
	virtual uint GetIndexCount();

	virtual glm::vec3* MapVertices();
	virtual void UnmapVertices();
	virtual uint GetVertexCount();

	virtual void UpdateInstanceBuffer(uint i, size_t size, void* data);

private:
	void Destroy();
	void UpdateGLBuffers(const MeshAttribute& attribute);
	int CalculateVBOCount(const MeshAttribute& attribute);

private:
	enum BufferIndex {
		IndexBuffer,
		VertexBuffer,
		InstanceBuffer0,
		InstanceBuffer1,
		BufferIndexCount,
	};

private:
	VAOPointer vao_;
	MeshTopology topology_;
	uint bufferIndexes_[BufferIndexCount];

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
