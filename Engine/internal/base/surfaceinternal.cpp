#include "tools/math2.h"
#include "surfaceinternal.h"
#include "internal/base/materialinternal.h"

MeshInternal::MeshInternal() :ObjectInternal(ObjectTypeMesh)
	, vertexCount_(0), baseVertex_(0), baseIndex_(0)
	, meshTopology_(MeshTopologyTriangles) {
}

void MeshInternal::SetTriangles(uint vertexCount, uint baseVertex, uint baseIndex) {
	vertexCount_ = vertexCount;
	baseVertex_ = baseVertex;
	baseIndex_ = baseIndex;
}

void MeshInternal::GetTriangles(uint& vertexCount, uint& baseVertex, uint& baseIndex) {
	vertexCount = vertexCount_;
	baseVertex = baseVertex_;
	baseIndex = baseIndex_;
}

SurfaceInternal::SurfaceInternal()
	: ObjectInternal(ObjectTypeSurface), indexBuffer_(0) {
	memset(instanceBuffer_, 0, sizeof(instanceBuffer_));
	vao_.Initialize();
}

SurfaceInternal::~SurfaceInternal() {
	Destroy();
}

void SurfaceInternal::Destroy() {
	meshes_.clear();
}

void SurfaceInternal::SetAttribute(const SurfaceAttribute& value) {
	vao_.Bind();
	UpdateGLBuffers(value);
	vao_.Unbind();
}

void SurfaceInternal::UpdateGLBuffers(const SurfaceAttribute& attribute) {
	int vboCount = CalculateVBOCount(attribute);
	vao_.CreateVBOs(vboCount);

	uint vboIndex = 0;

	if (!attribute.positions.empty()) {
		vao_.SetBuffer(vboIndex, GL_ARRAY_BUFFER, attribute.positions, GL_STATIC_DRAW);
		vao_.SetVertexDataSource(vboIndex, VertexAttribPosition, 3, GL_FLOAT, false, 0, 0);
		++vboIndex;
	}

	if (!attribute.texCoords.empty()) {
		vao_.SetBuffer(vboIndex, GL_ARRAY_BUFFER, attribute.texCoords, GL_STATIC_DRAW);
		vao_.SetVertexDataSource(vboIndex, VertexAttribTexCoord, 2, GL_FLOAT, false, 0, 0);
		++vboIndex;
	}

	if (!attribute.normals.empty()) {
		vao_.SetBuffer(vboIndex, GL_ARRAY_BUFFER, attribute.normals, GL_STATIC_DRAW);
		vao_.SetVertexDataSource(vboIndex, VertexAttribNormal, 3, GL_FLOAT, false, 0, 0);
		++vboIndex;
	}

	if (!attribute.tangents.empty()) {
		vao_.SetBuffer(vboIndex, GL_ARRAY_BUFFER, attribute.tangents, GL_STATIC_DRAW);
		vao_.SetVertexDataSource(vboIndex, VertexAttribTangent, 3, GL_FLOAT, false, 0, 0);
		++vboIndex;
	}

	if (!attribute.blendAttrs.empty()) {
		vao_.SetBuffer(vboIndex, GL_ARRAY_BUFFER, attribute.blendAttrs, GL_STATIC_DRAW);

		vao_.SetVertexDataSource(vboIndex, VertexAttribBoneIndexes, 4, GL_INT, false, sizeof(BlendAttribute), 0);
		vao_.SetVertexDataSource(vboIndex, VertexAttribBoneWeights, 4, GL_FLOAT, false, sizeof(BlendAttribute), (sizeof(uint) * BlendAttribute::Quality));
		++vboIndex;
	}

	if (!attribute.indexes.empty()) {
		vao_.SetBuffer(vboIndex, GL_ELEMENT_ARRAY_BUFFER, attribute.indexes, GL_STATIC_DRAW);
		indexBuffer_ = vboIndex++;
	}
	
	if (attribute.color.count != 0) {
		vao_.SetBuffer(vboIndex, GL_ARRAY_BUFFER, attribute.color.count * sizeof(glm::vec4), nullptr, GL_STREAM_DRAW);
		vao_.SetVertexDataSource(vboIndex, VertexAttribInstanceColor, 4, GL_FLOAT, false, 0, 0, attribute.color.divisor);
		instanceBuffer_[0] = vboIndex++;
	}

	if (attribute.geometry.count != 0) {
		vao_.SetBuffer(vboIndex, GL_ARRAY_BUFFER, attribute.geometry.count * sizeof(glm::vec4), nullptr, GL_STREAM_DRAW);
		vao_.SetVertexDataSource(vboIndex, VertexAttribInstanceGeometry, 4, GL_FLOAT, false, 0, 0, attribute.geometry.divisor);
		instanceBuffer_[1] = vboIndex++;
	}

	Assert(vboIndex == vboCount);
}

int SurfaceInternal::CalculateVBOCount(const SurfaceAttribute& attribute) {
	int count = 0;
	count += int(!attribute.positions.empty());
	count += int(!attribute.normals.empty());
	count += int(!attribute.texCoords.empty());
	count += int(!attribute.tangents.empty());
	count += int(!attribute.blendAttrs.empty());
	count += int(!attribute.indexes.empty());
	count += int(attribute.color.count != 0);
	count += int(attribute.geometry.count != 0);

	return count;
}

void SurfaceInternal::Bind() {
	vao_.Bind();
	vao_.BindBuffer(indexBuffer_);
}

void SurfaceInternal::Unbind() {
	vao_.Unbind();
	vao_.UnbindBuffer(indexBuffer_);
}

void SurfaceInternal::UpdateInstanceBuffer(uint i, size_t size, void* data) {
	AssertX(i < CountOf(instanceBuffer_), "index out of range");
	vao_.UpdateBuffer(instanceBuffer_[i], 0, size, data);
}
