#pragma once

#include <set>
#include <vector>

#include "mesh.h"
#include "tools/event.h"
#include "componentinternal.h"
#include "internal/base/vertexarray.h"
#include "internal/base/objectinternal.h"
#include "internal/base/globjectmaintainer.h"

class SubMeshInternal : public ObjectInternal {
public:
	SubMeshInternal();

public:
	void SetTriangleBias(const TriangleBias& value) { bias_ = value; }
	const TriangleBias& GetTriangleBias() const { return bias_; }

private:
	TriangleBias bias_;
};

class MeshInternal : public ObjectInternal, public GLObjectMaintainer {
public:
	MeshInternal(Context* context);
	~MeshInternal();

public:
	void CreateStorage();
	void SetAttribute(Mesh* self, const MeshAttribute& value);

	//void SetBounds(const Bounds& value) { bounds_ = value; }
	//const Bounds& GetBounds() const { return bounds_; }

	void Bind();
	void Unbind();
	void ShareStorage(Mesh* other);

	void AddSubMesh(SubMesh* subMesh);
	uint GetSubMeshCount() { return subMeshes_.size(); }
	SubMesh* GetSubMesh(uint index) { return subMeshes_[index].get(); }
	void RemoveSubMesh(uint index);

	MeshTopology GetTopology() { return storage_->topology; }
	uint GetNativePointer() const { return storage_->vao.GetNativePointer(); }

	const uint* MapIndexes();
	void UnmapIndexes();
	uint GetIndexCount();

	const Vector3* MapVertices();
	void UnmapVertices();
	uint GetVertexCount();

	void UpdateInstanceBuffer(uint i, size_t size, void* data);

public:
	enum BufferIndex {
		IndexBuffer,
		VertexBuffer,
		InstanceBuffer0,
		InstanceBuffer1,
		BufferIndexCount,
	};

	struct Storage {
		Storage(Context* context);

		VertexArray vao;
		MeshTopology topology;
		uint bufferIndexes[BufferIndexCount];

		event<> modified;
	};

	Storage* GetStorage() { return storage_.get(); }

protected:
	virtual void OnContextDestroyed();

private:
	void Destroy();
	void ClearAttribute(MeshAttribute& attribute);
	void UpdateGLBuffers(const MeshAttribute& attribute);
	int CalculateVBOCount(const MeshAttribute& attribute);
	void ApplyAttribute(const MeshAttribute& attribute);

//protected:
//	Bounds bounds_;

private:
	bool meshDirty_ = false;
	MeshAttribute attribute_;

	std::vector<ref_ptr<SubMesh>> subMeshes_;
	std::shared_ptr<Storage> storage_;
};

class MeshProviderInternal : public ComponentInternal {
public:
	MeshProviderInternal(ObjectType type);
	~MeshProviderInternal();

public:
	Mesh* GetMesh() { return mesh_.get(); }
	void SetMesh(Mesh* value);

private:
	void OnMeshModified();

private:
	ref_ptr<Mesh> mesh_;
};

class TextMeshInternal : public MeshProviderInternal {
public:
	TextMeshInternal();
	~TextMeshInternal();

public:
	virtual void Update(float deltaTime);

public:
	void SetText(const std::string& value);
	std::string GetText() { return text_; }

	int GetUpdateStrategy() { return UpdateStrategyRendering; }

	void SetFont(Font* value);
	Font* GetFont() { return font_.get(); }

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
	ref_ptr<Font> font_;
	bool meshDirty_;

	std::string text_;
};

class MeshFilterInternal : public MeshProviderInternal {
public:
	MeshFilterInternal();

public:
	int GetUpdateStrategy() { return UpdateStrategyNone; }
};
