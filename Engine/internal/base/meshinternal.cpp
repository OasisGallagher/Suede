#include "resources.h"
#include "tools/math2.h"
#include "tools/string.h"
#include "meshinternal.h"

SubMeshInternal::SubMeshInternal() :ObjectInternal(ObjectTypeSubMesh) {
}

MeshInternal::MeshInternal() : MeshInternal(ObjectTypeMesh) {
}

MeshInternal::MeshInternal(ObjectType type)
	: ObjectInternal(type) {
	memset(bufferIndexes_, 0, sizeof(bufferIndexes_));
}

MeshInternal::~MeshInternal() {
	Destroy();
}

void MeshInternal::Destroy() {
	subMeshes_.clear();
}

void MeshInternal::SetAttribute(const MeshAttribute& value) {
	topology_ = value.topology;

	if (!vao_) {
		vao_.reset(MEMORY_CREATE(VAO));
	}

	vao_->Bind();
	UpdateGLBuffers(value);
	vao_->Unbind();
}

void MeshInternal::UpdateGLBuffers(const MeshAttribute& attribute) {
	int vboCount = CalculateVBOCount(attribute);
	if (vboCount == 0) {
		Debug::LogWarning("empty mesh attribute");
		return;
	}

	// TODO: update vbo instead.
	vao_->CreateVBOs(vboCount);

	uint vboIndex = 0;

	if (!attribute.positions.empty()) {
		vao_->SetBuffer(vboIndex, GL_ARRAY_BUFFER, attribute.positions, GL_STATIC_DRAW);
		vao_->SetVertexDataSource(vboIndex, VertexAttribPosition, 3, GL_FLOAT, false, 0, 0);
		bufferIndexes_[VertexBuffer] = vboIndex++;
	}

	if (!attribute.texCoords.empty()) {
		vao_->SetBuffer(vboIndex, GL_ARRAY_BUFFER, attribute.texCoords, GL_STATIC_DRAW);
		vao_->SetVertexDataSource(vboIndex, VertexAttribTexCoord, 2, GL_FLOAT, false, 0, 0);
		++vboIndex;
	}

	if (!attribute.normals.empty()) {
		vao_->SetBuffer(vboIndex, GL_ARRAY_BUFFER, attribute.normals, GL_STATIC_DRAW);
		vao_->SetVertexDataSource(vboIndex, VertexAttribNormal, 3, GL_FLOAT, false, 0, 0);
		++vboIndex;
	}

	if (!attribute.tangents.empty()) {
		vao_->SetBuffer(vboIndex, GL_ARRAY_BUFFER, attribute.tangents, GL_STATIC_DRAW);
		vao_->SetVertexDataSource(vboIndex, VertexAttribTangent, 3, GL_FLOAT, false, 0, 0);
		++vboIndex;
	}

	if (!attribute.blendAttrs.empty()) {
		vao_->SetBuffer(vboIndex, GL_ARRAY_BUFFER, attribute.blendAttrs, GL_STATIC_DRAW);

		vao_->SetVertexDataSource(vboIndex, VertexAttribBoneIndexes, 4, GL_INT, false, sizeof(BlendAttribute), 0);
		vao_->SetVertexDataSource(vboIndex, VertexAttribBoneWeights, 4, GL_FLOAT, false, sizeof(BlendAttribute), (sizeof(uint) * BlendAttribute::Quality));
		++vboIndex;
	}

	if (!attribute.indexes.empty()) {
		vao_->SetBuffer(vboIndex, GL_ELEMENT_ARRAY_BUFFER, attribute.indexes, GL_STATIC_DRAW);
		bufferIndexes_[IndexBuffer] = vboIndex++;
	}
	
	if (attribute.color.count != 0) {
		vao_->SetBuffer(vboIndex, GL_ARRAY_BUFFER, attribute.color.count * sizeof(glm::vec4), nullptr, GL_STREAM_DRAW);
		vao_->SetVertexDataSource(vboIndex, VertexAttribInstanceColor, 4, GL_FLOAT, false, 0, 0, attribute.color.divisor);
		bufferIndexes_[InstanceBuffer0] = vboIndex++;
	}

	if (attribute.geometry.count != 0) {
		vao_->SetBuffer(vboIndex, GL_ARRAY_BUFFER, attribute.geometry.count * sizeof(glm::vec4), nullptr, GL_STREAM_DRAW);
		vao_->SetVertexDataSource(vboIndex, VertexAttribInstanceGeometry, 4, GL_FLOAT, false, 0, 0, attribute.geometry.divisor);
		bufferIndexes_[InstanceBuffer1] = vboIndex++;
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

void MeshInternal::ShareStorage(Mesh other) {
	MeshInternal* ptr = dynamic_cast<MeshInternal*>(other.get());
	vao_ = ptr->vao_;
	topology_ = ptr->topology_;
	memcpy(bufferIndexes_, ptr->bufferIndexes_, sizeof(bufferIndexes_));
}

void MeshInternal::AddSubMesh(SubMesh subMesh) {
	subMeshes_.push_back(subMesh);
	subMesh->SetMesh(dsp_cast<Mesh>(shared_from_this()));
}

void MeshInternal::Bind() {
	if (vao_->GetVBOCount() != 0) {
		vao_->Bind();
		vao_->BindBuffer(bufferIndexes_[IndexBuffer]);
	}
}

void MeshInternal::Unbind() {
	if (vao_->GetVBOCount() != 0) {
		vao_->Unbind();
		vao_->UnbindBuffer(bufferIndexes_[IndexBuffer]);
	}
}

void MeshInternal::RemoveSubMesh(uint index) {
	SubMesh subMesh = subMeshes_[index];
	subMeshes_.erase(subMeshes_.begin() + index);
	subMesh->SetMesh(nullptr);
}

uint* MeshInternal::MapIndexes() {
	return (uint*)vao_->MapBuffer(bufferIndexes_[IndexBuffer]);
}

void MeshInternal::UnmapIndexes() {
	vao_->UnmapBuffer(bufferIndexes_[IndexBuffer]);
}

uint MeshInternal::GetIndexCount() {
	return vao_->GetBufferSize(bufferIndexes_[IndexBuffer]) / sizeof(uint);
}

glm::vec3* MeshInternal::MapVertices() {
	return (glm::vec3*)vao_->MapBuffer(bufferIndexes_[VertexBuffer]);
}

void MeshInternal::UnmapVertices() {
	vao_->UnmapBuffer(bufferIndexes_[VertexBuffer]);
}

uint MeshInternal::GetVertexCount() {
	return vao_->GetBufferSize(bufferIndexes_[VertexBuffer]) / sizeof(glm::vec3);
}


void MeshInternal::UpdateInstanceBuffer(uint i, size_t size, void* data) {
	if (i >= 2) {
		Debug::LogError("index out of range");
		return;
	}

	vao_->UpdateBuffer(bufferIndexes_[InstanceBuffer0 + i], 0, size, data);
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
	// TODO: rebuild mesh twice with SetFont and SetFontSize.
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

	if (GetSubMeshCount() != 0) {
		RemoveSubMesh(0);
	}

	SubMesh subMesh = NewSubMesh();
	AddSubMesh(subMesh);

	TriangleBias bias{ attribute.indexes.size() };
	subMesh->SetTriangleBias(bias);

	SetAttribute(attribute);
}

void TextMeshInternal::InitializeMeshAttribute(MeshAttribute& attribute, const std::wstring& wtext) {
	const uint space = 2;
	const float scale = 0.08f;

	attribute.topology = MeshTopologyTriangles;

	uint cap = 6 * wtext.length();
	attribute.positions.reserve(cap);
	attribute.texCoords.reserve(cap);
	attribute.indexes.reserve(cap);

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
