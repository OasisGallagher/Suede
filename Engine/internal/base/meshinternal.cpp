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
}

MeshInternal::~MeshInternal() {
	Destroy();
}

void MeshInternal::Destroy() {
	subMeshes_.clear();
}

void MeshInternal::CreateStorage() {
	storage_.reset(MEMORY_CREATE(Storage));
}

void MeshInternal::SetAttribute(const MeshAttribute& value) {
	if (!storage_) {
		storage_.reset(MEMORY_CREATE(Storage));
	}

	storage_->vao.Initialize();
	storage_->topology = value.topology;
	UpdateGLBuffers(value);
}

void MeshInternal::UpdateGLBuffers(const MeshAttribute& attribute) {
	int vboCount = CalculateVBOCount(attribute);
	if (vboCount == 0) {
		Debug::LogWarning("empty mesh attribute");
		return;
	}

	// TODO: update vbo instead.
	storage_->vao.CreateVBOs(vboCount);

	storage_->vao.Bind();

	uint vboIndex = 0;
	
	if (!attribute.positions.empty()) {
		storage_->vao.SetBuffer(vboIndex, GL_ARRAY_BUFFER, attribute.positions, GL_STATIC_DRAW);
		storage_->vao.SetVertexDataSource(vboIndex, VertexAttribPosition, 3, GL_FLOAT, false, 0, 0);
		storage_->bufferIndexes[VertexBuffer] = vboIndex++;
	}

	if (!attribute.texCoords.empty()) {
		storage_->vao.SetBuffer(vboIndex, GL_ARRAY_BUFFER, attribute.texCoords, GL_STATIC_DRAW);
		storage_->vao.SetVertexDataSource(vboIndex, VertexAttribTexCoord, 2, GL_FLOAT, false, 0, 0);
		++vboIndex;
	}

	if (!attribute.normals.empty()) {
		storage_->vao.SetBuffer(vboIndex, GL_ARRAY_BUFFER, attribute.normals, GL_STATIC_DRAW);
		storage_->vao.SetVertexDataSource(vboIndex, VertexAttribNormal, 3, GL_FLOAT, false, 0, 0);
		++vboIndex;
	}

	if (!attribute.tangents.empty()) {
		storage_->vao.SetBuffer(vboIndex, GL_ARRAY_BUFFER, attribute.tangents, GL_STATIC_DRAW);
		storage_->vao.SetVertexDataSource(vboIndex, VertexAttribTangent, 3, GL_FLOAT, false, 0, 0);
		++vboIndex;
	}

	if (!attribute.blendAttrs.empty()) {
		storage_->vao.SetBuffer(vboIndex, GL_ARRAY_BUFFER, attribute.blendAttrs, GL_STATIC_DRAW);

		storage_->vao.SetVertexDataSource(vboIndex, VertexAttribBoneIndexes, 4, GL_INT, false, sizeof(BlendAttribute), 0);
		storage_->vao.SetVertexDataSource(vboIndex, VertexAttribBoneWeights, 4, GL_FLOAT, false, sizeof(BlendAttribute), (sizeof(uint) * BlendAttribute::Quality));
		++vboIndex;
	}

	if (!attribute.indexes.empty()) {
		storage_->vao.SetBuffer(vboIndex, GL_ELEMENT_ARRAY_BUFFER, attribute.indexes, GL_STATIC_DRAW);
		storage_->bufferIndexes[IndexBuffer] = vboIndex++;
	}

	if (attribute.color.count != 0) {
		storage_->vao.SetBuffer(vboIndex, GL_ARRAY_BUFFER, attribute.color.count * sizeof(glm::vec4), nullptr, GL_STREAM_DRAW);
		storage_->vao.SetVertexDataSource(vboIndex, VertexAttribInstanceColor, 4, GL_FLOAT, false, 0, 0, attribute.color.divisor);
		storage_->bufferIndexes[InstanceBuffer0] = vboIndex++;
	}

	if (attribute.geometry.count != 0) {
		storage_->vao.SetBuffer(vboIndex, GL_ARRAY_BUFFER, attribute.geometry.count * sizeof(glm::vec4), nullptr, GL_STREAM_DRAW);
		storage_->vao.SetVertexDataSource(vboIndex, VertexAttribInstanceGeometry, 4, GL_FLOAT, false, 0, 0, attribute.geometry.divisor);
		storage_->bufferIndexes[InstanceBuffer1] = vboIndex++;
	}

	storage_->vao.Unbind();
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
	if (!ptr->storage_) {
		Debug::LogError("empty storage");
		return;
	}

	storage_ = ptr->storage_;
}

void MeshInternal::AddSubMesh(SubMesh subMesh) {
	subMeshes_.push_back(subMesh);
	subMesh->SetMesh(dsp_cast<Mesh>(shared_from_this()));
}

void MeshInternal::Bind() {
	if (storage_->vao.GetVBOCount() != 0) {
		storage_->vao.Bind();
		storage_->vao.BindBuffer(storage_->bufferIndexes[IndexBuffer]);
	}
}

void MeshInternal::Unbind() {
	if (storage_->vao.GetVBOCount() != 0) {
		storage_->vao.Unbind();
		storage_->vao.UnbindBuffer(storage_->bufferIndexes[IndexBuffer]);
	}
}

void MeshInternal::RemoveSubMesh(uint index) {
	SubMesh subMesh = subMeshes_[index];
	subMeshes_.erase(subMeshes_.begin() + index);
	subMesh->SetMesh(nullptr);
}

uint* MeshInternal::MapIndexes() {
	return (uint*)storage_->vao.MapBuffer(storage_->bufferIndexes[IndexBuffer]);
}

void MeshInternal::UnmapIndexes() {
	storage_->vao.UnmapBuffer(storage_->bufferIndexes[IndexBuffer]);
}

uint MeshInternal::GetIndexCount() {
	return storage_->vao.GetBufferSize(storage_->bufferIndexes[IndexBuffer]) / sizeof(uint);
}

glm::vec3* MeshInternal::MapVertices() {
	return (glm::vec3*)storage_->vao.MapBuffer(storage_->bufferIndexes[VertexBuffer]);
}

void MeshInternal::UnmapVertices() {
	storage_->vao.UnmapBuffer(storage_->bufferIndexes[VertexBuffer]);
}

uint MeshInternal::GetVertexCount() {
	return storage_->vao.GetBufferSize(storage_->bufferIndexes[VertexBuffer]) / sizeof(glm::vec3);
}


void MeshInternal::UpdateInstanceBuffer(uint i, size_t size, void* data) {
	if (i >= BufferIndexCount - InstanceBuffer0) {
		Debug::LogError("index out of range");
		return;
	}

	storage_->vao.UpdateBuffer(storage_->bufferIndexes[InstanceBuffer0 + i], 0, size, data);
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

MeshInternal::Storage::Storage() {
	memset(bufferIndexes, 0, sizeof(bufferIndexes));
}
