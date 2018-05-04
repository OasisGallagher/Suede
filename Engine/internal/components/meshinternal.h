#pragma once

#include <vector>

#include "mesh.h"
#include "api/gl.h"
#include "componentinternal.h"
#include "internal/base/vertexarray.h"

class SubMeshInternal : public ISubMesh, public ObjectInternal {
	DEFINE_FACTORY_METHOD(SubMesh)

public:
	SubMeshInternal();

public:
	virtual void SetTriangleBias(const TriangleBias& value) { bias_ = value; }
	virtual const TriangleBias& GetTriangleBias() const { return bias_; }

private:
	TriangleBias bias_;
};

class MeshInternal : virtual public IMesh, public ComponentInternal {
	DEFINE_FACTORY_METHOD(Mesh)

public:
	MeshInternal();
	MeshInternal(ObjectType type);
	~MeshInternal();

public:
	virtual void CreateStorage();
	virtual void SetAttribute(const MeshAttribute& value);

	virtual void SetBounds(const Bounds& value) { bounds_ = value; }
	virtual const Bounds& GetBounds() const { return bounds_; }

	virtual void Bind();
	virtual void Unbind();
	virtual void ShareStorage(Mesh other);

	virtual void AddSubMesh(SubMesh subMesh);
	virtual int GetSubMeshCount() { return subMeshes_.size(); }
	virtual SubMesh GetSubMesh(uint index) { return subMeshes_[index]; }
	virtual void RemoveSubMesh(uint index);

	virtual MeshTopology GetTopology() { return storage_->topology; }
	virtual uint GetNativePointer() const { return storage_->vao.GetNativePointer(); }

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

	struct Storage {
		Storage();

		VertexArray vao;
		MeshTopology topology;
		uint bufferIndexes[BufferIndexCount];
	};

protected:
	Bounds bounds_;

private:
	std::vector<SubMesh> subMeshes_;
	std::shared_ptr<Storage> storage_;
};

class TextMeshInternal : public ITextMesh, public MeshInternal, public FontMaterialRebuiltListener {
	DEFINE_FACTORY_METHOD(TextMesh)

public:
	TextMeshInternal();
	~TextMeshInternal();

public:
	virtual void Update();

public:
	virtual void SetText(const std::string& value);
	virtual std::string GetText() { return text_; }

	virtual void SetFont(Font value);
	virtual Font GetFont() { return font_; }

	virtual void SetFontSize(uint value);
	virtual uint GetFontSize() { return size_; }

public:
	virtual void OnMaterialRebuilt();

private:
	void RebuildMesh();
	void RebuildUnicodeTextMesh(std::wstring wtext);

	void InitializeMeshAttribute(MeshAttribute& attribute, const std::wstring& wtext);

private:
	uint size_;
	Font font_;
	bool dirty_;
	std::string text_;
};
