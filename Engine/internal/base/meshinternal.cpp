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
	GetStorage().reset(MEMORY_CREATE(Storage));
}

void MeshInternal::SetAttribute(const MeshAttribute& value) {
	StoragePointer& storage = GetStorage();

	if (!storage) {
		storage.reset(MEMORY_CREATE(Storage));
	}

	storage->vao.Initialize();
	storage->topology = value.topology;
	UpdateGLBuffers(value);
}

void MeshInternal::UpdateGLBuffers(const MeshAttribute& attribute) {
	int vboCount = CalculateVBOCount(attribute);
	if (vboCount == 0) {
		Debug::LogWarning("empty mesh attribute");
		return;
	}

	StoragePointer& storage = GetStorage();

	// TODO: update vbo instead.
	storage->vao.CreateVBOs(vboCount);
	storage->vao.Bind();

	uint vboIndex = 0;
	
	if (!attribute.positions.empty()) {
		storage->vao.SetBuffer(vboIndex, GL_ARRAY_BUFFER, attribute.positions, GL_STATIC_DRAW);
		storage->vao.SetVertexDataSource(vboIndex, VertexAttribPosition, 3, GL_FLOAT, false, 0, 0);
		storage->bufferIndexes[VertexBuffer] = vboIndex++;
	}

	if (!attribute.texCoords.empty()) {
		storage->vao.SetBuffer(vboIndex, GL_ARRAY_BUFFER, attribute.texCoords, GL_STATIC_DRAW);
		storage->vao.SetVertexDataSource(vboIndex, VertexAttribTexCoord, 2, GL_FLOAT, false, 0, 0);
		++vboIndex;
	}

	if (!attribute.normals.empty()) {
		storage->vao.SetBuffer(vboIndex, GL_ARRAY_BUFFER, attribute.normals, GL_STATIC_DRAW);
		storage->vao.SetVertexDataSource(vboIndex, VertexAttribNormal, 3, GL_FLOAT, false, 0, 0);
		++vboIndex;
	}

	if (!attribute.tangents.empty()) {
		storage->vao.SetBuffer(vboIndex, GL_ARRAY_BUFFER, attribute.tangents, GL_STATIC_DRAW);
		storage->vao.SetVertexDataSource(vboIndex, VertexAttribTangent, 3, GL_FLOAT, false, 0, 0);
		++vboIndex;
	}

	if (!attribute.blendAttrs.empty()) {
		storage->vao.SetBuffer(vboIndex, GL_ARRAY_BUFFER, attribute.blendAttrs, GL_STATIC_DRAW);

		storage->vao.SetVertexDataSource(vboIndex, VertexAttribBoneIndexes, 4, GL_INT, false, sizeof(BlendAttribute), 0);
		storage->vao.SetVertexDataSource(vboIndex, VertexAttribBoneWeights, 4, GL_FLOAT, false, sizeof(BlendAttribute), (sizeof(uint) * BlendAttribute::Quality));
		++vboIndex;
	}

	if (!attribute.indexes.empty()) {
		storage->vao.SetBuffer(vboIndex, GL_ELEMENT_ARRAY_BUFFER, attribute.indexes, GL_STATIC_DRAW);
		storage->bufferIndexes[IndexBuffer] = vboIndex++;
	}

	if (attribute.color.count != 0) {
		storage->vao.SetBuffer(vboIndex, GL_ARRAY_BUFFER, attribute.color.count * sizeof(glm::vec4), nullptr, GL_STREAM_DRAW);
		storage->vao.SetVertexDataSource(vboIndex, VertexAttribInstanceColor, 4, GL_FLOAT, false, 0, 0, attribute.color.divisor);
		storage->bufferIndexes[InstanceBuffer0] = vboIndex++;
	}

	if (attribute.geometry.count != 0) {
		storage->vao.SetBuffer(vboIndex, GL_ARRAY_BUFFER, attribute.geometry.count * sizeof(glm::vec4), nullptr, GL_STREAM_DRAW);
		storage->vao.SetVertexDataSource(vboIndex, VertexAttribInstanceGeometry, 4, GL_FLOAT, false, 0, 0, attribute.geometry.divisor);
		storage->bufferIndexes[InstanceBuffer1] = vboIndex++;
	}

	storage->vao.Unbind();
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
	StoragePointer& storage2 = dynamic_cast<MeshInternal*>(other.get())->GetStorage();
	if (!storage2) {
		Debug::LogError("empty storage");
		return;
	}

	storage_ = storage2;
}

void MeshInternal::AddSubMesh(SubMesh subMesh) {
	subMeshes_.push_back(subMesh);
	subMesh->SetMesh(suede_dynamic_cast<Mesh>(shared_from_this()));
}

void MeshInternal::Bind() {
	StoragePointer& storage = GetStorage();
	if (storage->vao.GetVBOCount() != 0) {
		storage->vao.Bind();
		storage->vao.BindBuffer(storage->bufferIndexes[IndexBuffer]);
	}
}

void MeshInternal::Unbind() {
	StoragePointer& storage = GetStorage();
	if (storage->vao.GetVBOCount() != 0) {
		storage->vao.Unbind();
		storage->vao.UnbindBuffer(storage->bufferIndexes[IndexBuffer]);
	}
}

void MeshInternal::RemoveSubMesh(uint index) {
	SubMesh subMesh = subMeshes_[index];
	subMeshes_.erase(subMeshes_.begin() + index);
	subMesh->SetMesh(nullptr);
}

uint* MeshInternal::MapIndexes() {
	StoragePointer& storage = GetStorage();
	return (uint*)storage->vao.MapBuffer(storage->bufferIndexes[IndexBuffer]);
}

void MeshInternal::UnmapIndexes() {
	StoragePointer& storage = GetStorage();
	storage->vao.UnmapBuffer(storage->bufferIndexes[IndexBuffer]);
}

uint MeshInternal::GetIndexCount() {
	StoragePointer& storage = GetStorage();
	return storage->vao.GetBufferSize(storage->bufferIndexes[IndexBuffer]) / sizeof(uint);
}

glm::vec3* MeshInternal::MapVertices() {
	StoragePointer& storage = GetStorage();
	return (glm::vec3*)storage->vao.MapBuffer(storage->bufferIndexes[VertexBuffer]);
}

void MeshInternal::UnmapVertices() {
	StoragePointer& storage = GetStorage();
	storage->vao.UnmapBuffer(storage->bufferIndexes[VertexBuffer]);
}

uint MeshInternal::GetVertexCount() {
	StoragePointer& storage = GetStorage();
	return storage->vao.GetBufferSize(storage->bufferIndexes[VertexBuffer]) / sizeof(glm::vec3);
}


void MeshInternal::UpdateInstanceBuffer(uint i, size_t size, void* data) {
	if (i >= BufferIndexCount - InstanceBuffer0) {
		Debug::LogError("index out of range");
		return;
	}

	StoragePointer& storage = GetStorage();
	storage->vao.UpdateBuffer(storage->bufferIndexes[InstanceBuffer0 + i], 0, size, data);
}

#define RebuildStorageIfDirty()	if (dirty_) { RebuildStorage(); } else (void)0

TextMeshInternal::TextMeshInternal() : MeshInternal(ObjectTypeTextMesh), dirty_(false) {
}

StoragePointer& TextMeshInternal::GetStorage() {
	RebuildStorageIfDirty();
	return MeshInternal::GetStorage();
}

void TextMeshInternal::SetText(const std::string& value) {
	if (text_ != value) {
		text_ = value;
		dirty_ = true;
	}
}

void TextMeshInternal::SetFont(Font value) {
	if (font_ != value) {
		font_ = value;
		dirty_ = true;
	}
}

void TextMeshInternal::SetFontSize(uint value) {
	// TODO: rebuild mesh twice with SetFont and SetFontSize.
	if (size_ != value) {
		size_ = value;
		dirty_ = true;
	}
}

void TextMeshInternal::AddSubMesh(SubMesh subMesh) {
	RebuildStorageIfDirty();
	MeshInternal::AddSubMesh(subMesh);
}

int TextMeshInternal::GetSubMeshCount() {
	RebuildStorageIfDirty(); 
	return MeshInternal::GetSubMeshCount();
}

SubMesh TextMeshInternal::GetSubMesh(uint index) {
	RebuildStorageIfDirty();
	return MeshInternal::GetSubMesh(index);
}

void TextMeshInternal::RemoveSubMesh(uint index) {
	RebuildStorageIfDirty();
	MeshInternal::RemoveSubMesh(index);
}

void TextMeshInternal::RebuildStorage() {
	dirty_ = false;

	if (!text_.empty()) {
		if (GetSubMeshCount() == 0) {
			AddSubMesh(NewSubMesh());
		}

		BuildTextMesh(text_);
	}
}

void TextMeshInternal::BuildTextMesh(const std::string& text) {
	std::wstring wtext = String::MultiBytesToWideString(text);
	font_->Require(wtext);

	MeshAttribute attribute;

	InitializeMeshAttribute(attribute, wtext);

	SubMesh subMesh = GetSubMesh(0);

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

Storage::Storage() {
	memset(bufferIndexes, 0, sizeof(bufferIndexes));
}
