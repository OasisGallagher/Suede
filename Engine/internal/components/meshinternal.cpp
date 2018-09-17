#include "resources.h"
#include "tools/math2.h"
#include "tools/string.h"
#include "meshinternal.h"
#include "internal/base/vertexattrib.h"

RTTI_CLASS_DEFINITION(ITextMesh, IComponent)
RTTI_CLASS_DEFINITION(IMeshFilter, IComponent)

SubMeshInternal::SubMeshInternal() :ObjectInternal(ObjectType::SubMesh) {
}

MeshInternal::MeshInternal() : MeshInternal(ObjectType::Mesh) {
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
	storage_.reset(MEMORY_NEW(Storage));
}

void MeshInternal::SetAttribute(const MeshAttribute& value) {
	if (!storage_) {
		storage_.reset(MEMORY_NEW(Storage));
	}

	storage_->vao.Initialize();
	storage_->topology = value.topology;
	UpdateGLBuffers(value);
}

void MeshInternal::UpdateGLBuffers(const MeshAttribute& attribute) {
	int vboCount = CalculateVBOCount(attribute);
	if (vboCount == 0) {
		Debug::LogWarning("empty mesh attribute.");
		return;
	}

	// SUEDE TODO: update vbo instead.
	storage_->vao.CreateVertexBuffers(vboCount);

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
	MeshInternal* ptr = InternalPtr(other);
	if (!ptr->storage_) {
		Debug::LogError("empty storage");
		return;
	}

	storage_ = ptr->storage_;
}

void MeshInternal::AddSubMesh(SubMesh subMesh) {
	subMeshes_.push_back(subMesh);
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
	VERIFY_INDEX(i, BufferIndexCount - InstanceBuffer0, NOARG);
	storage_->vao.UpdateBuffer(storage_->bufferIndexes[InstanceBuffer0 + i], 0, size, data);
}

TextMeshInternal::TextMeshInternal() : ComponentInternal(ObjectType::TextMesh), dirty_(false) {
	mesh_ = NewMesh();
	mesh_->AddSubMesh(NewSubMesh());
}

TextMeshInternal::~TextMeshInternal() {
	if (font_) {
		font_->RemoveMaterialRebuiltListener(this);
	}
}

void TextMeshInternal::SetText(const std::string& value) {
	if (text_ != value) {
		text_ = value;
		dirty_ = true;
	}
}

void TextMeshInternal::SetFont(Font value) {
	if (font_ == value) { return; }

	if (font_) {
		font_->RemoveMaterialRebuiltListener(this);
	}

	if (value) {
		value->AddMaterialRebuiltListener(this);
	}

	font_ = value;
	dirty_ = true;
}

void TextMeshInternal::SetFontSize(uint value) {
	if (size_ != value) {
		size_ = value;
		dirty_ = true;
	}
}

void TextMeshInternal::RenderingUpdate() {
	if (dirty_) {
		RebuildMesh();
	}
}

void TextMeshInternal::OnMaterialRebuilt() {
	dirty_ = true;
}

void TextMeshInternal::RebuildMesh() {
	if (!text_.empty()) {
		std::wstring wtext = String::MultiBytesToWideString(text_);
		RebuildUnicodeTextMesh(wtext);
	}

	dirty_ = false;
}

void TextMeshInternal::RebuildUnicodeTextMesh(std::wstring wtext) {
	font_->Require(wtext);

	MeshAttribute attribute;
	InitializeMeshAttribute(attribute, wtext);

	SubMesh subMesh = mesh_->GetSubMesh(0);
	TriangleBias bias{ attribute.indexes.size() };
	subMesh->SetTriangleBias(bias);

	mesh_->SetAttribute(attribute);
}

void TextMeshInternal::InitializeMeshAttribute(MeshAttribute& attribute, const std::wstring& wtext) {
	const uint space = 2;
	const float scale = 0.08f;

	attribute.topology = MeshTopology::Triangles;

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

		attribute.texCoords.push_back(glm::vec2(info.texCoord.xy));
		attribute.texCoords.push_back(glm::vec2(info.texCoord.zy));
		attribute.texCoords.push_back(glm::vec2(info.texCoord.xw));

		attribute.texCoords.push_back(glm::vec2(info.texCoord.xw));
		attribute.texCoords.push_back(glm::vec2(info.texCoord.zy));
		attribute.texCoords.push_back(glm::vec2(info.texCoord.zw));

		x += info.width;
		x += space;
		for (int j = 0; j < 6; ++j) {
			attribute.indexes.push_back(6 * i + j);
		}
	}

	glm::vec3 min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::lowest());
	for (uint i = 0; i < attribute.positions.size(); ++i) {
		min = glm::min(min, attribute.positions[i]);
		max = glm::max(max, attribute.positions[i]);
	}

	Bounds bounds;
	bounds.SetMinMax(min, max);
	mesh_->SetBounds(bounds);

	GetGameObject()->RecalculateBounds(RecalculateBoundsFlagsSelf | RecalculateBoundsFlagsParent);
}

MeshInternal::Storage::Storage() {
	memset(bufferIndexes, 0, sizeof(bufferIndexes));
}

MeshFilterInternal::MeshFilterInternal()
	: ComponentInternal(ObjectType::MeshFilter) {
}

void MeshFilterInternal::SetMesh(Mesh value) {
	if (!value) {
		Debug::LogError("invalid mesh value.");
		return;
	}

	mesh_ = value;
}

Mesh MeshFilterInternal::GetMesh() {
	if (!mesh_) { mesh_ = NewMesh(); }
	return mesh_;
}
