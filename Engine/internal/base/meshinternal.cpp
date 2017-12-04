#include "math2.h"
#include "debug.h"
#include "resources.h"
#include "tools/string.h"
#include "meshinternal.h"
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
	if (i >= CountOf(instanceBuffer_)) {
		Debug::LogError("index out of range");
		return;
	}

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
	attribute.color.count = attribute.color.divisor = 0;
	attribute.geometry.count = attribute.geometry.divisor = 0;

	InitializeMeshAttribute(attribute, wtext);

	SubMesh subMesh = NewSubMesh();
	uint indexCount = attribute.indexes.back() + 1;
	subMesh->SetTriangles(indexCount, 0, 0);
	AddSubMesh(subMesh);

	SetAttribute(attribute);
}

void TextMeshInternal::InitializeMeshAttribute(MeshAttribute& attribute, const std::wstring& wtext) {
	const uint space = 2;
	const float scale = 0.08f;

	attribute.topology = MeshTopologyTriangles;
	uint x = 0;
	for (int i = 0; i < wtext.length(); ++i) {
		CharacterInfo info;
		if (!font_->GetCharacterInfo(wtext[i], &info)) {
			continue;
		}

		// lb, rb, lt, rt.
		attribute.positions.push_back(scale * glm::vec3(x, info.height / -2.f, 0));
		attribute.positions.push_back(scale * glm::vec3(x + info.width, info.height / -2.f, 0));
		attribute.positions.push_back(scale * glm::vec3(x, info.height / 2.f, 0));

		attribute.positions.push_back(scale * glm::vec3(x, info.height / 2.f, 0));
		attribute.positions.push_back(scale * glm::vec3(x + info.width, info.height / -2.f, 0));
		attribute.positions.push_back(scale * glm::vec3(x + info.width, info.height / 2.f, 0));

		attribute.texCoords.push_back(glm::vec2(info.texCoord.x, info.texCoord.y));
		attribute.texCoords.push_back(glm::vec2(info.texCoord.z, info.texCoord.y));
		attribute.texCoords.push_back(glm::vec2(info.texCoord.x, info.texCoord.w));

		attribute.texCoords.push_back(glm::vec2(info.texCoord.x, info.texCoord.w));
		attribute.texCoords.push_back(glm::vec2(info.texCoord.z, info.texCoord.y));
		attribute.texCoords.push_back(glm::vec2(info.texCoord.z, info.texCoord.w));

		x += info.width;
		x += space;
		for (int j = 0; j < 6; ++j) {
			attribute.indexes.push_back(6 * i + j);
		}
	}
}
