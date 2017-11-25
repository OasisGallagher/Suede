#include "tools/math2.h"
#include "tools/string.h"
#include "meshinternal.h"
#include "internal/resources/resources.h"
#include "internal/base/materialinternal.h"

SubMeshInternal::SubMeshInternal() :ObjectInternal(ObjectTypeSubMesh)
	, indexCount_(0), baseVertex_(0), baseIndex_(0) {
}

void SubMeshInternal::SetTriangles(uint indexCount, uint baseVertex, uint baseIndex) {
	indexCount_ = indexCount;
	baseVertex_ = baseVertex;
	baseIndex_ = baseIndex;
}

void SubMeshInternal::GetTriangles(uint& indexCount, uint& baseVertex, uint& baseIndex) {
	indexCount = indexCount_;
	baseVertex = baseVertex_;
	baseIndex = baseIndex_;
}

MeshInternal::MeshInternal() : MeshInternal(ObjectTypeMesh) {
}

MeshInternal::MeshInternal(ObjectType type)
	: ObjectInternal(type), indexBuffer_(0), topology_(MeshTopologyTriangles) {
	memset(instanceBuffer_, 0, sizeof(instanceBuffer_));
}

MeshInternal::~MeshInternal() {
	Destroy();
}

void MeshInternal::Destroy() {
	subMeshes_.clear();
}

void MeshInternal::SetAttribute(const MeshAttribute& value) {
	vao_.Bind();
	UpdateGLBuffers(value);
	vao_.Unbind();

	topology_ = value.topology;
}

void MeshInternal::UpdateGLBuffers(const MeshAttribute& attribute) {
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

int MeshInternal::CalculateVBOCount(const MeshAttribute& attribute) {
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

void MeshInternal::Bind() {
	vao_.Bind();
	vao_.BindBuffer(indexBuffer_);
}

void MeshInternal::Unbind() {
	vao_.Unbind();
	vao_.UnbindBuffer(indexBuffer_);
}

void MeshInternal::UpdateInstanceBuffer(uint i, size_t size, void* data) {
	AssertX(i < CountOf(instanceBuffer_), "index out of range");
	vao_.UpdateBuffer(instanceBuffer_[i], 0, size, data);
}

TextMeshInternal::TextMeshInternal() : MeshInternal(ObjectTypeTextMesh) {
}

void TextMeshInternal::SetText(const std::string& value) {
	if (text_ != value) {
		text_ = value;
		RebuildMesh();
	}
}

void TextMeshInternal::SetFont(Font value) {
	if (font_ != value) {
		font_ = value;
		RebuildMesh();
	}
}

void TextMeshInternal::SetFontSize(uint value) {
	if (size_ != value) {
		size_ = value;
		RebuildMesh();
	}
}

void TextMeshInternal::RebuildMesh() {
	if (text_.empty()) { return; }
	std::wstring wtext = String::MultiBytesToWideString(text_);
	font_->Require(wtext);

	MeshAttribute attribute;
	InitializeMeshAttribute(attribute, wtext);

	SubMesh subMesh = CREATE_OBJECT(SubMesh);
	uint indexCount = attribute.indexes.back() + 1;
	subMesh->SetTriangles(indexCount, 0, 0);
	AddSubMesh(subMesh);

	SetAttribute(attribute);
}

void TextMeshInternal::InitializeMeshAttribute(MeshAttribute& attribute, const std::wstring& wtext) {
	const float quadScale = 1.f;
	MeshAttribute quad;
	Resources::GetPrimitiveAttribute(PrimitiveTypeQuad, quadScale, quad);
	attribute.topology = quad.topology;

	uint vc = quad.positions.size(), ic = quad.indexes.size();

	for (int i = 0; i < wtext.length(); ++i) {
		for (int j = 0; j < vc; ++j) {
			glm::vec3 pos = quad.positions[j];
			pos.x += i * quadScale;
			attribute.positions.push_back(pos);
			//attribute.texCoords.push_back(quad.texCoords[j]);
		}
		// lb, rb, lt, rt.
		glm::vec4 coord = font_->GetTexCoord(wtext[i]);
		attribute.texCoords.push_back(glm::vec2(coord.x, coord.y));
		attribute.texCoords.push_back(glm::vec2(coord.z, coord.y));
		attribute.texCoords.push_back(glm::vec2(coord.x, coord.w));
		attribute.texCoords.push_back(glm::vec2(coord.z, coord.w));
		
		for (int j = 0; j < ic; ++j) {
			attribute.indexes.push_back(ic * i + j);
		}
	}
}
