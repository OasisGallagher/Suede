#include "resources.h"
#include "math/mathf.h"
#include "tools/string.h"
#include "meshinternal.h"
#include "geometryutility.h"

#include "internal/base/context.h"
#include "internal/base/vertexattrib.h"

SubMesh::SubMesh() : Object(new SubMeshInternal) {}
const TriangleBias& SubMesh::GetTriangleBias() const { return _suede_dptr()->GetTriangleBias(); }
void SubMesh::SetTriangleBias(const TriangleBias& value) { _suede_dptr()->SetTriangleBias(value); }

void MeshAttribute::GetPrimitiveAttribute(PrimitiveType type, MeshAttribute& attribute, float scale) {
	if (type == PrimitiveType::Quad) {
		attribute.topology = MeshTopology::TriangleStripe;

		attribute.positions.assign({
			Vector3(-0.5f * scale, -0.5f * scale, 0.f),
			Vector3(0.5f * scale, -0.5f * scale, 0.f),
			Vector3(-0.5f * scale,  0.5f * scale, 0.f),
			Vector3(0.5f * scale,  0.5f * scale, 0.f),
		});

		attribute.texCoords[0].assign({
			Vector2(0.f, 0.f),
			Vector2(1.f, 0.f),
			Vector2(0.f, 1.f),
			Vector2(1.f, 1.f),
		});

		attribute.indexes.assign({ 0, 1, 2, 3 });
	}
	else if (type == PrimitiveType::Cube) {
		attribute.topology = MeshTopology::Triangles;
		GeometryUtility::GetCuboidCoordinates(attribute.positions, Vector3(0), Vector3(1), &attribute.indexes);

		for (int i = 0; i < attribute.positions.size(); ++i) {
			attribute.positions[i] *= scale;
		}

		attribute.normals.assign({
			Vector3(0.333333f, 0.666667f, -0.666667f),
			Vector3(-0.816497f, 0.408248f, -0.408248f),
			Vector3(-0.333333f, 0.666667f, 0.666667f),
			Vector3(0.816497f, 0.408248f, 0.408248f),
			Vector3(0.666667f, -0.666667f, -0.333333f),
			Vector3(-0.408248f, -0.408248f, -0.816497f),
			Vector3(-0.666667f, -0.666667f, 0.333333f),
			Vector3(0.408248f, -0.408248f, 0.816497f)
		});

		attribute.texCoords[0].assign({
			Vector2(0.f, 1.f),
			Vector2(1.f, 1.f),
			Vector2(0.f, 1.f),
			Vector2(1.f, 1.f),
			Vector2(0.f, 0.f),
			Vector2(1.f, 0.f),
			Vector2(0.f, 0.f),
			Vector2(1.f, 0.f),
		});
	}
}

Mesh::Mesh() : Object(new MeshInternal(Context::GetCurrent())) {}

ref_ptr<Mesh> Mesh::FromAttribute(const MeshAttribute& attribute) {
	Mesh* mesh = new Mesh();
	mesh->SetAttribute(attribute);

	SubMesh* subMesh = new SubMesh();
	subMesh->SetTriangleBias(TriangleBias{ (uint)attribute.indexes.size() });

	mesh->AddSubMesh(subMesh);
	return mesh;
}

Mesh* Mesh::GetPrimitive(PrimitiveType type) {
	static ref_ptr<Mesh> primitiveCache[PrimitiveType::size()];
	if (!primitiveCache[type]) {
		primitiveCache[type] = CreatePrimitive((PrimitiveType)type, 1);
	}

	SUEDE_ASSERT(type >= 0 && type < PrimitiveType::size());
	return primitiveCache[type].get();
}

ref_ptr<Mesh> Mesh::CreatePrimitive(PrimitiveType type, float scale) {
	MeshAttribute attribute;
	MeshAttribute::GetPrimitiveAttribute(type, attribute, scale);

	return Mesh::FromAttribute(attribute);
}

ref_ptr<Mesh> Mesh::CreateInstancedPrimitive(PrimitiveType type, float scale, const InstanceAttribute& color, const InstanceAttribute& geometry) {
	MeshAttribute attribute;
	MeshAttribute::GetPrimitiveAttribute(type, attribute, scale);
	attribute.color = color;
	attribute.geometry = geometry;

	return Mesh::FromAttribute(attribute);
}

void Mesh::CreateStorage() { _suede_dptr()->CreateStorage(); }
void Mesh::SetAttribute(const MeshAttribute& value) { _suede_dptr()->SetAttribute(value); }
//const Bounds& IMesh::GetBounds() const { return _suede_dptr()->GetBounds(); }
//void IMesh::SetBounds(const Bounds& value) { _suede_dptr()->SetBounds(value); }

void Mesh::AddSubMesh(SubMesh* subMesh) { _suede_dptr()->AddSubMesh(subMesh); }
uint Mesh::GetSubMeshCount() { return _suede_dptr()->GetSubMeshCount(); }
SubMesh* Mesh::GetSubMesh(uint index) { return _suede_dptr()->GetSubMesh(index); }
void Mesh::RemoveSubMesh(uint index) { _suede_dptr()->RemoveSubMesh(index); }
MeshTopology Mesh::GetTopology() { return _suede_dptr()->GetTopology(); }
uint Mesh::GetNativePointer() const { return _suede_dptr()->GetNativePointer(); }
const uint* Mesh::MapIndexes() { return _suede_dptr()->MapIndexes(); }
void Mesh::UnmapIndexes() { _suede_dptr()->UnmapIndexes(); }
uint Mesh::GetIndexCount() { return _suede_dptr()->GetIndexCount(); }
const Vector3* Mesh::MapVertices() { return _suede_dptr()->MapVertices(); }
void Mesh::UnmapVertices() { _suede_dptr()->UnmapVertices(); }
uint Mesh::GetVertexCount() { return _suede_dptr()->GetVertexCount(); }
void Mesh::Bind() { _suede_dptr()->Bind(); }
void Mesh::Unbind() { _suede_dptr()->Unbind(); }
void Mesh::ShareStorage(Mesh* other) { _suede_dptr()->ShareStorage(other); }
void Mesh::UpdateInstanceBuffer(uint i, size_t size, void* data) { _suede_dptr()->UpdateInstanceBuffer(i, size, data); }

MeshProvider::MeshProvider(void* d) : Component(d) {}
Mesh* MeshProvider::GetMesh() { return _suede_dptr()->GetMesh(); }
TextMesh::TextMesh() : MeshProvider(new TextMeshInternal) {}
void TextMesh::SetText(const std::string& value) { _suede_dptr()->SetText(value); }
std::string TextMesh::GetText() { return _suede_dptr()->GetText(); }
void TextMesh::SetFont(Font* value) { _suede_dptr()->SetFont(value); }
Font* TextMesh::GetFont() { return _suede_dptr()->GetFont(); }
void TextMesh::SetFontSize(uint value) { _suede_dptr()->SetFontSize(value); }
uint TextMesh::GetFontSize() { return _suede_dptr()->GetFontSize(); }

MeshFilter::MeshFilter() : MeshProvider(new MeshFilterInternal) {}
void MeshFilter::SetMesh(Mesh* value) { _suede_dptr()->SetMesh(value); }

SUEDE_DEFINE_COMPONENT_INTERNAL(MeshProvider, Component)
SUEDE_DEFINE_COMPONENT_INTERNAL(TextMesh, MeshProvider)
SUEDE_DEFINE_COMPONENT_INTERNAL(MeshFilter, MeshProvider)

SubMeshInternal::SubMeshInternal() :ObjectInternal(ObjectType::SubMesh) {
}

MeshInternal::MeshInternal(Context* context)
	: ObjectInternal(ObjectType::Mesh), context_(context) {
	context_->destroyed.subscribe(this, &MeshInternal::OnContextDestroyed);
}

MeshInternal::~MeshInternal() {
	Destroy();
	if (context_ != nullptr) {
		context_->destroyed.unsubscribe(this);
	}
}

void MeshInternal::Destroy() {
	subMeshes_.clear();
	storage_.reset();
}

void MeshInternal::OnContextDestroyed() {
	Destroy();
	context_ = nullptr;
}

void MeshInternal::CreateStorage() {
	if (!storage_) { storage_.reset(new Storage(context_)); }
}

void MeshInternal::SetAttribute(const MeshAttribute& value) {
	CreateStorage();

	if (context_->InThisThread()) {
		SyncMeshAttribute(value);
	}
	else {
		attribute_ = value;
		meshDirty_ = true;
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

void MeshInternal::ShareStorage(Mesh* other) {
	MeshInternal* ptr = _suede_rptr(other);
	if (ptr->storage_) {
		storage_ = ptr->storage_;
	}
	else {
		Debug::LogError("empty storage");
	}
}

void MeshInternal::AddSubMesh(SubMesh* subMesh) {
	subMeshes_.push_back(subMesh);
}

void MeshInternal::Bind() {
	if (meshDirty_) {
		SyncMeshAttribute(attribute_);
		ClearAttribute(attribute_);
	}

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

void MeshInternal::SyncMeshAttribute(const MeshAttribute& attribute) {
	storage_->vao.Initialize();
	storage_->topology = attribute.topology;
	UpdateGLBuffers(attribute);
	meshDirty_ = false;

	storage_->modified.raise();
}

void MeshInternal::ClearAttribute(MeshAttribute& attribute) {
	attribute.positions.clear();
	attribute.normals.clear();

	for (int i = 0; i < MeshAttribute::TexCoordsCount; ++i) {
		attribute.texCoords[i].clear();
	}

	attribute.tangents.clear();
	attribute.blendAttrs.clear();
	attribute.indexes.clear();
}

#define Impl(mesh)	((MeshInternal*)mesh->d_)

MeshProviderInternal::MeshProviderInternal(ObjectType type) : ComponentInternal(type) {
}

MeshProviderInternal::~MeshProviderInternal() {
	auto storage = Impl(mesh_)->GetStorage();
	if (storage != nullptr) {
		storage->modified.unsubscribe(this);
	}
}

void MeshProviderInternal::SetMesh(Mesh* value) {
	if (mesh_ != nullptr) {
		Impl(mesh_)->GetStorage()->modified.unsubscribe(this);
	}

	if (value != nullptr) {
		Impl(value)->GetStorage()->modified.subscribe(this, &MeshProviderInternal::OnMeshModified);
	}

	mesh_ = value;
}

void MeshProviderInternal::OnMeshModified() {
	GetGameObject()->SendMessage(GameObjectMessageMeshModified, nullptr);
}

TextMeshInternal::TextMeshInternal() : MeshProviderInternal(ObjectType::TextMesh), meshDirty_(false) {
	Mesh* mesh = new Mesh();
	mesh->CreateStorage();

	SetMesh(mesh);
	GetMesh()->AddSubMesh(new SubMesh());
}

TextMeshInternal::~TextMeshInternal() {
	if (font_) {
		font_->materialRebuilt.unsubscribe(this);
	}
}

void TextMeshInternal::SetText(const std::string& value) {
	if (text_ != value) {
		text_ = value;
		meshDirty_ = true;
	}
}

void TextMeshInternal::SetFont(Font* value) {
	if (font_ == value) { return; }

	if (font_) {
		font_->materialRebuilt.unsubscribe(this);
	}

	if (value) {
		value->materialRebuilt.subscribe(this, &TextMeshInternal::OnMaterialRebuilt);
	}

	font_ = value;
	meshDirty_ = true;
}

void TextMeshInternal::SetFontSize(uint value) {
	if (size_ != value) {
		size_ = value;
		meshDirty_ = true;
	}
}

void TextMeshInternal::Update(float deltaTime) {
	if (meshDirty_) {
		RebuildMesh();
	}
}

void TextMeshInternal::OnMaterialRebuilt() {
	meshDirty_ = true;
}

void TextMeshInternal::RebuildMesh() {
	if (!text_.empty()) {
		std::wstring wtext = String::MultiBytesToWideString(text_);
		RebuildUnicodeTextMesh(wtext);
	}

	meshDirty_ = false;
}

void TextMeshInternal::RebuildUnicodeTextMesh(std::wstring wtext) {
	font_->Require(wtext);

	MeshAttribute attribute;
	InitializeMeshAttribute(attribute, wtext);

	SubMesh* subMesh = GetMesh()->GetSubMesh(0);
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

MeshInternal::Storage::Storage(Context* context) : topology(MeshTopology::Triangles), vao(context) {
	memset(bufferIndexes, 0, sizeof(bufferIndexes));
}

MeshFilterInternal::MeshFilterInternal() : MeshProviderInternal(ObjectType::MeshFilter) {
}
