#include "resources.h"
#include "math/mathf.h"
#include "tools/string.h"
#include "meshinternal.h"
#include "internal/base/vertexattrib.h"

ISubMesh::ISubMesh() : IObject(MEMORY_NEW(SubMeshInternal)) {}
const TriangleBias& ISubMesh::GetTriangleBias() const { return _suede_dptr()->GetTriangleBias(); }
void ISubMesh::SetTriangleBias(const TriangleBias& value) { _suede_dptr()->SetTriangleBias(value); }

IMesh::IMesh() : IObject(MEMORY_NEW(MeshInternal)) {}
void IMesh::CreateStorage() { _suede_dptr()->CreateStorage(); }
void IMesh::SetAttribute(const MeshAttribute& value) { _suede_dptr()->SetAttribute(value); }
//const Bounds& IMesh::GetBounds() const { return _suede_dptr()->GetBounds(); }
//void IMesh::SetBounds(const Bounds& value) { _suede_dptr()->SetBounds(value); }

void IMesh::AddSubMesh(SubMesh subMesh) { _suede_dptr()->AddSubMesh(subMesh); }
uint IMesh::GetSubMeshCount() { return _suede_dptr()->GetSubMeshCount(); }
SubMesh IMesh::GetSubMesh(uint index) { return _suede_dptr()->GetSubMesh(index); }
IMesh::Enumerable IMesh::GetSubMeshes() { return _suede_dptr()->GetSubMeshes(); }
void IMesh::RemoveSubMesh(uint index) { _suede_dptr()->RemoveSubMesh(index); }
MeshTopology IMesh::GetTopology() { return _suede_dptr()->GetTopology(); }
uint IMesh::GetNativePointer() const { return _suede_dptr()->GetNativePointer(); }
const uint* IMesh::MapIndexes() { return _suede_dptr()->MapIndexes(); }
void IMesh::UnmapIndexes() { _suede_dptr()->UnmapIndexes(); }
uint IMesh::GetIndexCount() { return _suede_dptr()->GetIndexCount(); }
const Vector3* IMesh::MapVertices() { return _suede_dptr()->MapVertices(); }
void IMesh::UnmapVertices() { _suede_dptr()->UnmapVertices(); }
uint IMesh::GetVertexCount() { return _suede_dptr()->GetVertexCount(); }
void IMesh::Bind() { _suede_dptr()->Bind(); }
void IMesh::Unbind() { _suede_dptr()->Unbind(); }
void IMesh::ShareStorage(Mesh other) { _suede_dptr()->ShareStorage(other); }
void IMesh::UpdateInstanceBuffer(uint i, size_t size, void* data) { _suede_dptr()->UpdateInstanceBuffer(i, size, data); }

IMeshProvider::IMeshProvider(void* d) : IComponent(d) {}
Mesh IMeshProvider::GetMesh() { return _suede_dptr()->GetMesh(); }
ITextMesh::ITextMesh() : IMeshProvider(MEMORY_NEW(TextMeshInternal)) {}
void ITextMesh::SetText(const std::string& value) { _suede_dptr()->SetText(value); }
std::string ITextMesh::GetText() { return _suede_dptr()->GetText(); }
void ITextMesh::SetFont(Font value) { _suede_dptr()->SetFont(value); }
Font ITextMesh::GetFont() { return _suede_dptr()->GetFont(); }
void ITextMesh::SetFontSize(uint value) { _suede_dptr()->SetFontSize(value); }
uint ITextMesh::GetFontSize() { return _suede_dptr()->GetFontSize(); }

IMeshFilter::IMeshFilter() : IMeshProvider(MEMORY_NEW(MeshFilterInternal)) {}
void IMeshFilter::SetMesh(Mesh value) { _suede_dptr()->SetMesh(value); }

SUEDE_DEFINE_COMPONENT_INTERNAL(MeshProvider, Component)
SUEDE_DEFINE_COMPONENT_INTERNAL(TextMesh, MeshProvider)
SUEDE_DEFINE_COMPONENT_INTERNAL(MeshFilter, MeshProvider)

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
	if (!storage_) { storage_.reset(MEMORY_NEW(Storage)); }
}

void MeshInternal::SetAttribute(const MeshAttribute& value) {
	CreateStorage();

	storage_->vao.Initialize();
	storage_->topology = value.topology;
	UpdateGLBuffers(value);

	for (IMeshModifiedListener* listener : storage_->listeners) {
		listener->OnMeshModified();
	}
}

void MeshInternal::UpdateGLBuffers(const MeshAttribute& attribute) {
	int vboCount = CalculateVBOCount(attribute);
	if (vboCount == 1) {
		Debug::LogWarning("empty mesh attribute.");
		return;
	}

	storage_->vao.CreateVertexBuffers(vboCount);
	storage_->vao.Bind();

	uint vboIndex = 0;
	
	if (!attribute.positions.empty()) {
		storage_->vao.SetBuffer(vboIndex, GL_ARRAY_BUFFER, attribute.positions, GL_STATIC_DRAW);
		storage_->vao.SetVertexDataSource(vboIndex, VertexAttribPosition, 3, GL_FLOAT, false, 0, 0);
		storage_->bufferIndexes[VertexBuffer] = vboIndex++;
	}

	for (int i = 0; i < MeshAttribute::TexCoordsCount; ++i) {
		if (!attribute.texCoords[i].empty()) {
			storage_->vao.SetBuffer(vboIndex, GL_ARRAY_BUFFER, attribute.texCoords[i], GL_STATIC_DRAW);
			storage_->vao.SetVertexDataSource(vboIndex, VertexAttribTexCoord0 + i, 2, GL_FLOAT, false, 0, 0);
			++vboIndex;
		}
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
		storage_->vao.SetBuffer(vboIndex, GL_ARRAY_BUFFER, attribute.color.count * sizeof(Vector4), nullptr, GL_STREAM_DRAW);
		storage_->vao.SetVertexDataSource(vboIndex, VertexAttribInstanceColor, 4, GL_FLOAT, false, 0, 0, attribute.color.divisor);
		storage_->bufferIndexes[InstanceBuffer0] = vboIndex++;
	}

	if (attribute.geometry.count != 0) {
		storage_->vao.SetBuffer(vboIndex, GL_ARRAY_BUFFER, attribute.geometry.count * sizeof(Vector4), nullptr, GL_STREAM_DRAW);
		storage_->vao.SetVertexDataSource(vboIndex, VertexAttribInstanceGeometry, 4, GL_FLOAT, false, 0, 0, attribute.geometry.divisor);
		storage_->bufferIndexes[InstanceBuffer1] = vboIndex++;
	}

	storage_->vao.Unbind();
}

int MeshInternal::CalculateVBOCount(const MeshAttribute& attribute) {
	int count = 1;	// for VertexAttribMatrixOffset.
	count += int(!attribute.positions.empty());
	
	for (int i = 0; i < MeshAttribute::TexCoordsCount; ++i) {
		count += int(!attribute.texCoords[i].empty());
	}

	count += int(!attribute.normals.empty());

	count += int(!attribute.tangents.empty());
	count += int(!attribute.blendAttrs.empty());
	count += int(!attribute.indexes.empty());
	count += int(attribute.color.count != 0);
	count += int(attribute.geometry.count != 0);

	return count;
}

void MeshInternal::ShareStorage(Mesh other) {
	MeshInternal* ptr = _suede_rptr(other);
	if (ptr->storage_) {
		storage_ = ptr->storage_; 
	}
	else {
		Debug::LogError("empty storage");
	}
}

void MeshInternal::AddMeshModifiedListener(IMeshModifiedListener* listener) {
	if (storage_) {
		storage_->listeners.insert(listener);
	}
	else {
		Debug::LogError("failed to add mesh modified listener, empty storage");
	}
}

void MeshInternal::RemoveMeshModifiedListener(IMeshModifiedListener* listener) {
	if (storage_) {
		storage_->listeners.erase(listener);
	}
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

const uint* MeshInternal::MapIndexes() {
	return (uint*)storage_->vao.MapBuffer(storage_->bufferIndexes[IndexBuffer]);
}

void MeshInternal::UnmapIndexes() {
	storage_->vao.UnmapBuffer(storage_->bufferIndexes[IndexBuffer]);
}

uint MeshInternal::GetIndexCount() {
	return storage_->vao.GetBufferSize(storage_->bufferIndexes[IndexBuffer]) / sizeof(uint);
}

const Vector3* MeshInternal::MapVertices() {
	return (Vector3*)storage_->vao.MapBuffer(storage_->bufferIndexes[VertexBuffer]);
}

void MeshInternal::UnmapVertices() {
	storage_->vao.UnmapBuffer(storage_->bufferIndexes[VertexBuffer]);
}

uint MeshInternal::GetVertexCount() {
	return storage_->vao.GetBufferSize(storage_->bufferIndexes[VertexBuffer]) / sizeof(Vector3);
}

void MeshInternal::UpdateInstanceBuffer(uint i, size_t size, void* data) {
	SUEDE_VERIFY_INDEX(i, BufferIndexCount - InstanceBuffer0, SUEDE_NOARG);
	storage_->vao.UpdateBuffer(storage_->bufferIndexes[InstanceBuffer0 + i], 0, size, data);
}

#define GetMeshInternal(mesh)	((mesh).get()->_rptr_impl<MeshInternal>())

MeshProviderInternal::MeshProviderInternal(ObjectType type) : ComponentInternal(type) {
}

MeshProviderInternal::~MeshProviderInternal() {
	GetMeshInternal(mesh_)->RemoveMeshModifiedListener(this);
}

void MeshProviderInternal::SetMesh(Mesh value) {
	if (mesh_ != nullptr) { GetMeshInternal(mesh_)->RemoveMeshModifiedListener(this); }
	if (value != nullptr) { GetMeshInternal(value)->AddMeshModifiedListener(this); }

	mesh_ = value;
}

void MeshProviderInternal::OnMeshModified() {
	GetGameObject()->SendMessage(GameObjectMessageMeshModified, nullptr);
}

TextMeshInternal::TextMeshInternal() : MeshProviderInternal(ObjectType::TextMesh), dirty_(false) {
	Mesh mesh = new IMesh();
	mesh->CreateStorage();

	SetMesh(mesh);
	GetMesh()->AddSubMesh(new ISubMesh());
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

void TextMeshInternal::Update() {
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

	SubMesh subMesh = GetMesh()->GetSubMesh(0);
	TriangleBias bias{ attribute.indexes.size() };
	subMesh->SetTriangleBias(bias);

	GetMesh()->SetAttribute(attribute);
}

void TextMeshInternal::InitializeMeshAttribute(MeshAttribute& attribute, const std::wstring& wtext) {
	const uint space = 2;
	const float scale = 0.08f;

	attribute.topology = MeshTopology::Triangles;

	uint cap = 6 * wtext.length();
	attribute.positions.reserve(cap);
	attribute.texCoords[0].reserve(cap);
	attribute.indexes.reserve(cap);

	float x = 0;
	for (int i = 0; i < wtext.length(); ++i) {
		CharacterInfo info;
		if (!font_->GetCharacterInfo(wtext[i], &info)) {
			continue;
		}

		// lb, rb, lt, rt.
		attribute.positions.push_back(scale * Vector3(x, info.height / -2.f, 0));
		attribute.positions.push_back(scale * Vector3(x + info.width, info.height / -2.f, 0));
		attribute.positions.push_back(scale * Vector3(x, info.height / 2.f, 0));

		attribute.positions.push_back(scale * Vector3(x, info.height / 2.f, 0));
		attribute.positions.push_back(scale * Vector3(x + info.width, info.height / -2.f, 0));
		attribute.positions.push_back(scale * Vector3(x + info.width, info.height / 2.f, 0));

		attribute.texCoords[0].push_back(Vector2(info.texCoord.x, info.texCoord.y));
		attribute.texCoords[0].push_back(Vector2(info.texCoord.z, info.texCoord.y));
		attribute.texCoords[0].push_back(Vector2(info.texCoord.x, info.texCoord.w));

		attribute.texCoords[0].push_back(Vector2(info.texCoord.x, info.texCoord.w));
		attribute.texCoords[0].push_back(Vector2(info.texCoord.z, info.texCoord.y));
		attribute.texCoords[0].push_back(Vector2(info.texCoord.z, info.texCoord.w));

		x += info.width;
		x += space;
		for (int j = 0; j < 6; ++j) {
			attribute.indexes.push_back(6 * i + j);
		}
	}

	Vector3 min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::lowest());
	for (uint i = 0; i < attribute.positions.size(); ++i) {
		min = Vector3::Min(min, attribute.positions[i]);
		max = Vector3::Max(max, attribute.positions[i]);
	}
}

MeshInternal::Storage::Storage() : topology(MeshTopology::Triangles) {
	memset(bufferIndexes, 0, sizeof(bufferIndexes));
}

MeshFilterInternal::MeshFilterInternal() : MeshProviderInternal(ObjectType::MeshFilter) {
}
