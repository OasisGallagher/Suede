#pragma once

#include <vector>
#include <wrappers/gl.h>

#include "mesh.h"
#include "vertexarray.h"
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

enum BufferIndex {
	IndexBuffer,
	VertexBuffer,
	InstanceBuffer0,
	InstanceBuffer1,
	BufferIndexCount,
};

struct Storage {
	Storage();

	VertexArray vao;
	MeshTopology topology;
	uint bufferIndexes[BufferIndexCount];
};

typedef std::shared_ptr<Storage> StoragePointer;

class SubMeshInternal : public ISubMesh, public ObjectInternal {
	DEFINE_FACTORY_METHOD(SubMesh)

public:
	SubMeshInternal();

	virtual void SetMesh(Mesh value) { mesh_ = value; }
	virtual Mesh GetMesh() { return mesh_.lock(); }

	virtual void SetTriangleBias(const TriangleBias& value) { bias_ = value; }
	virtual const TriangleBias& GetTriangleBias() const { return bias_; }

private:
	TriangleBias bias_;
	std::weak_ptr<Mesh::element_type> mesh_;
};

class MeshInternal : virtual public IMesh, public ObjectInternal {
	DEFINE_FACTORY_METHOD(Mesh)

public:
	MeshInternal();
	MeshInternal(ObjectType type);
	~MeshInternal();

public:
	virtual void CreateStorage();
	virtual void SetAttribute(const MeshAttribute& value);

	virtual void Bind();
	virtual void Unbind();
	virtual void ShareStorage(Mesh other);

	virtual void AddSubMesh(SubMesh subMesh);
	virtual int GetSubMeshCount() { return subMeshes_.size(); }
	virtual SubMesh GetSubMesh(uint index) { return subMeshes_[index]; }
	virtual void RemoveSubMesh(uint index);

	virtual MeshTopology GetTopology() { return GetStorage()->topology; }
	virtual uint GetNativePointer() { return GetStorage()->vao.GetNativePointer(); }

	virtual uint* MapIndexes();
	virtual void UnmapIndexes();
	virtual uint GetIndexCount();

	virtual glm::vec3* MapVertices();
	virtual void UnmapVertices();
	virtual uint GetVertexCount();

	virtual void UpdateInstanceBuffer(uint i, size_t size, void* data);

protected:
	virtual StoragePointer& GetStorage() { return storage_; }

private:
	void Destroy();
	void UpdateGLBuffers(const MeshAttribute& attribute);
	int CalculateVBOCount(const MeshAttribute& attribute);

private:
	std::vector<SubMesh> subMeshes_;
	std::shared_ptr<Storage> storage_;
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

	virtual void AddSubMesh(SubMesh subMesh);
	virtual int GetSubMeshCount();
	virtual SubMesh GetSubMesh(uint index);
	virtual void RemoveSubMesh(uint index);

protected:
	virtual StoragePointer& GetStorage();

private:
	void RebuildStorage();
	void BuildTextMesh(const std::string& text);

	void InitializeMeshAttribute(MeshAttribute& attribute, const std::wstring& wtext);

private:
	uint size_;
	Font font_;

	bool dirty_;
	std::string text_;
};
