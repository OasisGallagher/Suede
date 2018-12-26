#pragma once

#include <set>
#include <vector>

#include "mesh.h"
#include "../api/gl.h"
#include "componentinternal.h"
#include "internal/base/vertexarray.h"
#include "internal/base/objectinternal.h"

class SubMeshInternal : public ObjectInternal {
public:
	SubMeshInternal();

public:
	void SetTriangleBias(const TriangleBias& value) { bias_ = value; }
	const TriangleBias& GetTriangleBias() const { return bias_; }

private:
	TriangleBias bias_;
};

class IMeshModifiedListener {
public:
	virtual void OnMeshModified() = 0;
};

class MeshInternal : public ObjectInternal {
public:
	MeshInternal();
	MeshInternal(ObjectType type);
	~MeshInternal();

public:
	void CreateStorage();
	void SetAttribute(const MeshAttribute& value);

	//void SetBounds(const Bounds& value) { bounds_ = value; }
	//const Bounds& GetBounds() const { return bounds_; }

	void Bind();
	void Unbind();
	void ShareStorage(Mesh other);

	void AddMeshModifiedListener(IMeshModifiedListener* listener);
	void RemoveMeshModifiedListener(IMeshModifiedListener* listener);

	void AddSubMesh(SubMesh subMesh);
	uint GetSubMeshCount() { return subMeshes_.size(); }
	SubMesh GetSubMesh(uint index) { return subMeshes_[index]; }
	IMesh::Enumerable GetSubMeshes() { return IMesh::Enumerable(subMeshes_.begin(), subMeshes_.end()); }
	void RemoveSubMesh(uint index);

	MeshTopology GetTopology() { return storage_->topology; }
	uint GetNativePointer() const { return storage_->vao.GetNativePointer(); }

	const uint* MapIndexes();
	void UnmapIndexes();
	uint GetIndexCount();

	const glm::vec3* MapVertices();
	void UnmapVertices();
	uint GetVertexCount();

	void UpdateInstanceBuffer(uint i, size_t size, void* data);

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

		std::set<IMeshModifiedListener*> listeners;
	};

//protected:
//	Bounds bounds_;

private:
	std::vector<SubMesh> subMeshes_;
	std::shared_ptr<Storage> storage_;
};

class MeshProviderInternal : public ComponentInternal, public IMeshModifiedListener {
public:
	MeshProviderInternal(ObjectType type);
	~MeshProviderInternal();

public:
	Mesh GetMesh() { return mesh_; }
	void SetMesh(Mesh value);

public:
	virtual void OnMeshModified();

private:
	Mesh mesh_;
};

class TextMeshInternal : public MeshProviderInternal, public FontMaterialRebuiltListener {
public:
	TextMeshInternal();
	~TextMeshInternal();

public:
	virtual void Update();

public:
	void SetText(const std::string& value);
	std::string GetText() { return text_; }

	int GetUpdateStrategy() { return UpdateStrategyRendering; }

	void SetFont(Font value);
	Font GetFont() { return font_; }

	void SetFontSize(uint value);
	uint GetFontSize() { return size_; }

public:
	void OnMaterialRebuilt();

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

class MeshFilterInternal : public MeshProviderInternal {
public:
	MeshFilterInternal();

public:
	int GetUpdateStrategy() { return UpdateStrategyNone; }
};
