#pragma once

#include <set>
#include <bitset>
#include <vector>

#include "mesh.h"
#include "tools/event.h"
#include "componentinternal.h"
#include "internal/base/vertexarray.h"
#include "internal/base/objectinternal.h"
#include "internal/base/globjectmaintainer.h"

class GeometryInternal : public ObjectInternal {
public:
	GeometryInternal(Context* context);
	~GeometryInternal();

public:
	event<> modified;

	void SetTopology(MeshTopology value) { topology_ = value; }
	MeshTopology GetTopology() const { return topology_; }

	void UpdateInstanceBuffer(int index, size_t size, void* data);

	void SetVertices(const Vector3* values, int count);
	const Vector3* GetVertices() const { return vertices_.data(); }
	uint GetVertexCount() const { return vertices_.size(); }

	void SetNormals(const Vector3* values, int count);
	const Vector3* GetNormals() const { return normals_.data(); }
	uint GetNormalCount() const { return normals_.size(); }

	void SetTangents(const Vector3* values, int count);
	const Vector3* GetTangents() const { return tangents_.data(); }
	uint GetTangentCount() const { return tangents_.size(); }

	void SetTexCoords(int index, const Vector2* values, int count);
	const Vector2* GetTexCoords(int index) const;
	uint GetTexCoordCount(int index) const;

	void SetBlendAttributes(const BlendAttribute* values, int count);
	const BlendAttribute* GetBlendAttributes() const { return blendAttrs_.data(); }
	uint GetBlendAttributeCount() const { return blendAttrs_.size(); }

	void SetIndexes(const uint* values, int count);
	const uint* GetIndexes() const { return indexes_.data(); }
	uint GetIndexCount() const { return indexes_.size(); }

	void SetColorInstanceAttribute(const InstanceAttribute& value);
	void SetGeometryInstanceAttribute(const InstanceAttribute& value);

	void Bind();
	void Unbind();

private:
	void Apply();
	void ApplyInstanceBuffers();

private:
	enum BufferNames {
		Vertices,
		Normals,
		TexCoords,
		Tangents = TexCoords + Geometry::TexCoordsCount,
		BlendAttributes,
		Indexes,
		_InstanceBufferBegin,
		ColorInstances = _InstanceBufferBegin,
		GeometryInstances,
		_BufferCount,
	};

	std::bitset<_BufferCount> dirtyFlags_;
	MeshTopology topology_ = MeshTopology::Triangles;

	std::vector<Vector3> vertices_;
	std::vector<Vector3> normals_;
	std::vector<Vector2> texCoords_[Geometry::TexCoordsCount];
	std::vector<Vector3> tangents_;
	std::vector<BlendAttribute> blendAttrs_;
	std::vector<uint> indexes_;

	InstanceAttribute colorInstances_;
	InstanceAttribute geometryInstances_;

	struct {
		uint size = 0;
		std::unique_ptr<uchar[]> data;
	} instanceBuffers_[_BufferCount - _InstanceBufferBegin];

	Context* context_;
	VertexArray* vao_ = nullptr;
};

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
	void SetGeometry(Geometry* value);
	Geometry* GetGeometry();

	void Bind();
	void Unbind();

	void AddSubMesh(SubMesh* subMesh);
	void RemoveSubMesh(uint index);
	SubMesh* GetSubMesh(uint index) { return subMeshes_[index].get(); }
	uint GetSubMeshCount() { return subMeshes_.size(); }

	const Bounds& GetBounds();

protected:
	virtual void OnContextDestroyed();

private:
	void Destroy();
	void RecalculateBounds();

private:
	bool boundsDirty_ = false;
	Bounds bounds_;

	ref_ptr<Geometry> geometry_;
	std::vector<ref_ptr<SubMesh>> subMeshes_;
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
	void RebuildUnicodeTextMesh(const std::wstring& wtext);

	void InitializeGeometry(Geometry* geometry, const std::wstring& wtext);

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
