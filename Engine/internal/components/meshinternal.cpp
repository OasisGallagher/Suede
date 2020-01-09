#include "meshinternal.h"

#include "resources.h"
#include "geometryutility.h"

#include "internal/base/context.h"
#include "internal/base/vertexattrib.h"

Geometry::Geometry() : Object(new GeometryInternal(Context::GetCurrent())) {}
void Geometry::SetVertices(const Vector3* values, int count) { _suede_dptr()->SetVertices(values, count); }
const Vector3* Geometry::GetVertices() const { return _suede_dptr()->GetVertices(); }
uint Geometry::GetVertexCount() const { return _suede_dptr()->GetVertexCount(); }
void Geometry::SetNormals(const Vector3* values, int count) { _suede_dptr()->SetNormals(values, count); }
const Vector3* Geometry::GetNormals() const { return _suede_dptr()->GetNormals(); }
uint Geometry::GetNormalCount() const { return _suede_dptr()->GetNormalCount(); }
void Geometry::SetTangents(const Vector3* values, int count) { _suede_dptr()->SetTangents(values, count); }
const Vector3* Geometry::GetTangents() const { return _suede_dptr()->GetTangents(); }
uint Geometry::GetTangentCount() const { return _suede_dptr()->GetTangentCount(); }
void Geometry::SetTexCoords(int index, const Vector2* values, int count) { _suede_dptr()->SetTexCoords(index, values, count); }
const Vector2* Geometry::GetTexCoords(int index) const { return _suede_dptr()->GetTexCoords(index); }
uint Geometry::GetTexCoordCount(int index) const { return _suede_dptr()->GetTexCoordCount(index); }
void Geometry::SetBlendAttributes(const BlendAttribute* values, int count) { _suede_dptr()->SetBlendAttributes(values, count); }
const BlendAttribute* Geometry::GetBlendAttributes() const { return _suede_dptr()->GetBlendAttributes(); }
uint Geometry::GetBlendAttributeCount() const { return _suede_dptr()->GetBlendAttributeCount(); }
void Geometry::SetIndexes(const uint* values, int count) { _suede_dptr()->SetIndexes(values, count); }
const uint* Geometry::GetIndexes() const { return _suede_dptr()->GetIndexes(); }
uint Geometry::GetIndexCount() const { return _suede_dptr()->GetIndexCount(); }
void Geometry::SetTopology(MeshTopology value) { _suede_dptr()->SetTopology(value); }
MeshTopology Geometry::GetTopology() const { return _suede_dptr()->GetTopology(); }
void Geometry::SetColorInstanceAttribute(const InstanceAttribute& value) { _suede_dptr()->SetColorInstanceAttribute(value); }
void Geometry::SetGeometryInstanceAttribute(const InstanceAttribute& value) { _suede_dptr()->SetGeometryInstanceAttribute(value); }
void Geometry::UpdateInstanceBuffer(int index, size_t size, void* data) { _suede_dptr()->UpdateInstanceBuffer(index, size, data); }

Geometry* Geometry::GetPrimitive(PrimitiveType type) {
	static ref_ptr<Geometry> primitiveCache[PrimitiveType::size()];
	if (!primitiveCache[type]) {
		primitiveCache[type] = CreatePrimitive(type, 1);
	}

	SUEDE_ASSERT(type >= 0 && type < PrimitiveType::size());
	return primitiveCache[type].get();
}

ref_ptr<Geometry> Geometry::CreatePrimitive(PrimitiveType type, float scale) {
	ref_ptr<Geometry> geometry;
	if (type == PrimitiveType::Quad) {
		geometry = new Geometry();
		geometry->SetTopology(MeshTopology::TriangleStripe);

		auto vertices = {
			Vector3(-0.5f * scale, -0.5f * scale, 0.f),
			Vector3(0.5f * scale, -0.5f * scale, 0.f),
			Vector3(-0.5f * scale,  0.5f * scale, 0.f),
			Vector3(0.5f * scale,  0.5f * scale, 0.f),
		};

		geometry->SetVertices(vertices.begin(), vertices.size());

		auto texCoords = {
			Vector2(0.f, 0.f),
			Vector2(1.f, 0.f),
			Vector2(0.f, 1.f),
			Vector2(1.f, 1.f),
		};

		geometry->SetTexCoords(0, texCoords.begin(), texCoords.size());

		uint indexes[] = { 0, 1, 2, 3 };
		geometry->SetIndexes(indexes, SUEDE_COUNTOF(indexes));
	}
	else if (type == PrimitiveType::Cube) {
		geometry = new Geometry();
		geometry->SetTopology(MeshTopology::Triangles);

		std::vector<uint> indexes;
		std::vector<Vector3> vertices;
		GeometryUtility::GetCuboidCoordinates(vertices, Vector3(0), Vector3(1), &indexes);

		for (int i = 0; i < vertices.size(); ++i) {
			vertices[i] *= scale;
		}

		geometry->SetIndexes(indexes.data(), indexes.size());
		geometry->SetVertices(vertices.data(), vertices.size());

		auto normals = {
			Vector3(0.333333f, 0.666667f, -0.666667f),
			Vector3(-0.816497f, 0.408248f, -0.408248f),
			Vector3(-0.333333f, 0.666667f, 0.666667f),
			Vector3(0.816497f, 0.408248f, 0.408248f),
			Vector3(0.666667f, -0.666667f, -0.333333f),
			Vector3(-0.408248f, -0.408248f, -0.816497f),
			Vector3(-0.666667f, -0.666667f, 0.333333f),
			Vector3(0.408248f, -0.408248f, 0.816497f)
		};

		geometry->SetNormals(normals.begin(), normals.size());

		auto texCoords = {
			Vector2(0.f, 1.f),
			Vector2(1.f, 1.f),
			Vector2(0.f, 1.f),
			Vector2(1.f, 1.f),
			Vector2(0.f, 0.f),
			Vector2(1.f, 0.f),
			Vector2(0.f, 0.f),
			Vector2(1.f, 0.f),
		};

		geometry->SetTexCoords(0, texCoords.begin(), texCoords.size());
	}

	return geometry;
}

SubMesh::SubMesh() : Object(new SubMeshInternal) {}
const TriangleBias& SubMesh::GetTriangleBias() const { return _suede_dptr()->GetTriangleBias(); }
void SubMesh::SetTriangleBias(const TriangleBias& value) { _suede_dptr()->SetTriangleBias(value); }

Mesh::Mesh() : Object(new MeshInternal(Context::GetCurrent())) {}
void Mesh::SetGeometry(Geometry* value) { _suede_dptr()->SetGeometry(value); }
Geometry* Mesh::GetGeometry() { return _suede_dptr()->GetGeometry(); }
void Mesh::AddSubMesh(SubMesh* subMesh) { _suede_dptr()->AddSubMesh(subMesh); }
uint Mesh::GetSubMeshCount() { return _suede_dptr()->GetSubMeshCount(); }
SubMesh* Mesh::GetSubMesh(uint index) { return _suede_dptr()->GetSubMesh(index); }
void Mesh::RemoveSubMesh(uint index) { _suede_dptr()->RemoveSubMesh(index); }
void Mesh::Bind() { _suede_dptr()->Bind(); }
void Mesh::Unbind() { _suede_dptr()->Unbind(); }
const Bounds& Mesh::GetBounds() { return _suede_dptr()->GetBounds(); }
ref_ptr<Mesh> Mesh::FromGeometry(Geometry* geometry) {
	ref_ptr<Mesh> mesh = new Mesh();
	mesh->SetGeometry(geometry);

	SubMesh* subMesh = new SubMesh();
	subMesh->SetTriangleBias(TriangleBias{ geometry->GetIndexCount() });

	mesh->AddSubMesh(subMesh);
	return mesh;
}

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

SubMeshInternal::SubMeshInternal() : ObjectInternal(ObjectType::SubMesh) {
}

MeshInternal::MeshInternal(Context* context)
	: ObjectInternal(ObjectType::Mesh), GLObjectMaintainer(context) {
}

MeshInternal::~MeshInternal() {
	Destroy();
}

void MeshInternal::Destroy() {
	subMeshes_.clear();
	geometry_.reset();
}

void MeshInternal::RecalculateBounds() {
	SUEDE_ASSERT(geometry_);
	const Vector3* vertices = geometry_->GetVertices();
	const uint* indexes = geometry_->GetIndexes();

	Vector3 min(std::numeric_limits<float>::max());
	Vector3 max(std::numeric_limits<float>::lowest());

	for (ref_ptr<SubMesh>& subMesh : subMeshes_) {
		const TriangleBias& bias = subMesh->GetTriangleBias();

		for (int j = 0; j < bias.indexCount; j += 3) {
			uint index0 = indexes[bias.baseIndex + j] + bias.baseVertex;
			uint index1 = indexes[bias.baseIndex + j + 1] + bias.baseVertex;
			uint index2 = indexes[bias.baseIndex + j + 2] + bias.baseVertex;

			min = Vector3::Min(min, vertices[index0]);
			max = Vector3::Max(max, vertices[index0]);

			min = Vector3::Min(min, vertices[index1]);
			max = Vector3::Max(max, vertices[index1]);

			min = Vector3::Min(min, vertices[index2]);
			max = Vector3::Max(max, vertices[index2]);
		}
	}

	bounds_.SetMinMax(min, max);
	boundsDirty_ = false;
}

void MeshInternal::OnContextDestroyed() {
	Destroy();
	GLObjectMaintainer::OnContextDestroyed();
}

void MeshInternal::SetGeometry(Geometry* value) {
	SUEDE_ASSERT(value != nullptr);
	geometry_.reset(value);
	boundsDirty_ = true;
}

Geometry* MeshInternal::GetGeometry() {
	if (!geometry_) { geometry_ = new Geometry(); }
	return geometry_.get();
}

void MeshInternal::AddSubMesh(SubMesh* subMesh) {
	subMeshes_.push_back(subMesh);
	boundsDirty_ = true;
}

void MeshInternal::RemoveSubMesh(uint index) {
	subMeshes_.erase(subMeshes_.begin() + index);
	boundsDirty_ = true;
}

void MeshInternal::Bind() {
	SUEDE_ASSERT(geometry_);
	_suede_drptr(geometry_.get())->Bind();
}

void MeshInternal::Unbind() {
	SUEDE_ASSERT(geometry_);
	_suede_drptr(geometry_.get())->Unbind();
}

const Bounds& MeshInternal::GetBounds() {
	if (boundsDirty_) { RecalculateBounds(); }
	return bounds_;
}

MeshProviderInternal::MeshProviderInternal(ObjectType type) : ComponentInternal(type) {
}

MeshProviderInternal::~MeshProviderInternal() {
	auto geometry = _suede_drptr(mesh_->GetGeometry());
	if (geometry != nullptr) {
		geometry->modified.unsubscribe(this);
	}
}

void MeshProviderInternal::SetMesh(Mesh* value) {
	if (mesh_ != nullptr) {
		_suede_drptr(mesh_->GetGeometry())->modified.unsubscribe(this);
	}

	if (value != nullptr) {
		_suede_drptr(value->GetGeometry())->modified.subscribe(this, &MeshProviderInternal::OnMeshModified);
	}

	mesh_ = value;
}

void MeshProviderInternal::OnMeshModified() {
	GetGameObject()->SendMessage(GameObjectMessageMeshModified, nullptr);
}

TextMeshInternal::TextMeshInternal() : MeshProviderInternal(ObjectType::TextMesh), meshDirty_(false) {
	Mesh* mesh = new Mesh();

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

void TextMeshInternal::RebuildUnicodeTextMesh(const std::wstring& wtext) {
	font_->Require(wtext);

	ref_ptr<Geometry> geometry = new Geometry();
	InitializeGeometry(geometry.get(), wtext);

	SubMesh* subMesh = GetMesh()->GetSubMesh(0);
	TriangleBias bias{ geometry->GetIndexCount() };
	subMesh->SetTriangleBias(bias);

	GetMesh()->SetGeometry(geometry.get());
}

void TextMeshInternal::InitializeGeometry(Geometry* geometry, const std::wstring& wtext) {
	const uint space = 2;
	const float scale = 0.08f;

	geometry->SetTopology(MeshTopology::Triangles);

	uint cap = 6 * wtext.length();
	float x = 0;
	std::vector<uint> indexes;
	std::vector<Vector3> vertices;
	std::vector<Vector2> texCoords;
	for (int i = 0; i < wtext.length(); ++i) {
		CharacterInfo info;
		if (!font_->GetCharacterInfo(wtext[i], &info)) {
			continue;
		}

		// lb, rb, lt, rt.
		vertices.push_back(scale * Vector3(x, info.height / -2.f, 0));
		vertices.push_back(scale * Vector3(x + info.width, info.height / -2.f, 0));
		vertices.push_back(scale * Vector3(x, info.height / 2.f, 0));

		vertices.push_back(scale * Vector3(x, info.height / 2.f, 0));
		vertices.push_back(scale * Vector3(x + info.width, info.height / -2.f, 0));
		vertices.push_back(scale * Vector3(x + info.width, info.height / 2.f, 0));

		texCoords.push_back(Vector2(info.texCoord.x, info.texCoord.y));
		texCoords.push_back(Vector2(info.texCoord.z, info.texCoord.y));
		texCoords.push_back(Vector2(info.texCoord.x, info.texCoord.w));

		texCoords.push_back(Vector2(info.texCoord.x, info.texCoord.w));
		texCoords.push_back(Vector2(info.texCoord.z, info.texCoord.y));
		texCoords.push_back(Vector2(info.texCoord.z, info.texCoord.w));

		x += info.width;
		x += space;
		for (int j = 0; j < 6; ++j) {
			indexes.push_back(6 * i + j);
		}
	}

	geometry->SetVertices(vertices.data(), vertices.size());
	geometry->SetTexCoords(0, texCoords.data(), texCoords.size());
	geometry->SetIndexes(indexes.data(), indexes.size());

	Vector3 min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::lowest());
	for (uint i = 0; i < vertices.size(); ++i) {
		min = Vector3::Min(min, vertices[i]);
		max = Vector3::Max(max, vertices[i]);
	}
}

MeshFilterInternal::MeshFilterInternal() : MeshProviderInternal(ObjectType::MeshFilter) {
}

GeometryInternal::GeometryInternal(Context* context) : ObjectInternal(ObjectType::SubMesh), context_(context) {
}

GeometryInternal::~GeometryInternal() {
	delete vao_;
}

void GeometryInternal::Bind() {
	if (vao_ == nullptr) {
		vao_ = new VertexArray(context_);
		vao_->CreateVertexBuffers(dirtyFlags_.size());
	}

	if (dirtyFlags_.any()) {
		Apply();
	}

	ApplyInstanceBuffers();

	if (vao_->GetVBOCount() != 0) {
		vao_->Bind();
		vao_->BindBuffer(Indexes);
	}
}

void GeometryInternal::Unbind() {
	if (vao_->GetVBOCount() != 0) {
		vao_->Unbind();
		vao_->UnbindBuffer(Indexes);
	}
}

void GeometryInternal::UpdateInstanceBuffer(int index, size_t size, void* data) {
	SUEDE_ASSERT(index >= 0 && index < SUEDE_COUNTOF(instanceBuffers_));
	auto& ib = instanceBuffers_[index];
	ib.data.reset(new uchar[size]);
	memcpy(ib.data.get(), data, size);
	ib.size = size;
}

void GeometryInternal::SetVertices(const Vector3* values, int count) {
	vertices_.assign(values, values + count);
	dirtyFlags_[Vertices] = true;
}

void GeometryInternal::SetNormals(const Vector3* values, int count) {
	normals_.assign(values, values + count);
	dirtyFlags_[Normals] = true;
}

void GeometryInternal::SetTangents(const Vector3* values, int count) {
	tangents_.assign(values, values + count);
	dirtyFlags_[Tangents] = true;
}

void GeometryInternal::SetTexCoords(int index, const Vector2* values, int count) {
	SUEDE_ASSERT(index >= 0 && index < Geometry::TexCoordsCount);
	texCoords_[index].assign(values, values + count);
	dirtyFlags_[TexCoords + index] = true;
}

const Vector2* GeometryInternal::GetTexCoords(int index) const {
	SUEDE_ASSERT(index >= 0 && index < Geometry::TexCoordsCount);
	return texCoords_[index].data();
}

uint GeometryInternal::GetTexCoordCount(int index) const {
	SUEDE_ASSERT(index >= 0 && index < Geometry::TexCoordsCount);
	return texCoords_[index].size();
}

void GeometryInternal::SetBlendAttributes(const BlendAttribute* values, int count) {
	blendAttrs_.assign(values, values + count);
	dirtyFlags_[BlendAttributes] = true;
}

void GeometryInternal::SetIndexes(const uint* values, int count) {
	indexes_.assign(values, values + count);
	dirtyFlags_[Indexes] = true;
}

void GeometryInternal::SetColorInstanceAttribute(const InstanceAttribute& value) {
	colorInstances_ = value;
	dirtyFlags_[ColorInstances] = true;
}

void GeometryInternal::SetGeometryInstanceAttribute(const InstanceAttribute& value) {
	geometryInstances_ = value;
	dirtyFlags_[GeometryInstances] = true;
}

void GeometryInternal::Apply() {
	vao_->Bind();

	if (dirtyFlags_[Vertices]) {
		vao_->SetBuffer(Vertices, GL_ARRAY_BUFFER, vertices_, GL_STATIC_DRAW);
		vao_->SetVertexDataSource(Vertices, VertexAttribPosition, 3, GL_FLOAT, false, 0, 0);
	}

	for (int i = 0; i < Geometry::TexCoordsCount; ++i) {
		if (dirtyFlags_[TexCoords + i]) {
			vao_->SetBuffer(TexCoords + i, GL_ARRAY_BUFFER, texCoords_[i], GL_STATIC_DRAW);
			vao_->SetVertexDataSource(TexCoords + i, VertexAttribTexCoord0 + i, 2, GL_FLOAT, false, 0, 0);
		}
	}

	if (dirtyFlags_[Normals]) {
		vao_->SetBuffer(Normals, GL_ARRAY_BUFFER, normals_, GL_STATIC_DRAW);
		vao_->SetVertexDataSource(Normals, VertexAttribNormal, 3, GL_FLOAT, false, 0, 0);
	}

	if (dirtyFlags_[Tangents]) {
		vao_->SetBuffer(Tangents, GL_ARRAY_BUFFER, tangents_, GL_STATIC_DRAW);
		vao_->SetVertexDataSource(Tangents, VertexAttribTangent, 3, GL_FLOAT, false, 0, 0);
	}

	if (dirtyFlags_[BlendAttributes]) {
		vao_->SetBuffer(BlendAttributes, GL_ARRAY_BUFFER, blendAttrs_, GL_STATIC_DRAW);

		vao_->SetVertexDataSource(BlendAttributes, VertexAttribBoneIndexes, 4, GL_INT, false, sizeof(BlendAttribute), 0);
		vao_->SetVertexDataSource(BlendAttributes, VertexAttribBoneWeights, 4, GL_FLOAT, false, sizeof(BlendAttribute), (sizeof(uint) * BlendAttribute::Quality));
	}

	if (dirtyFlags_[Indexes]) {
		vao_->SetBuffer(Indexes, GL_ELEMENT_ARRAY_BUFFER, indexes_, GL_STATIC_DRAW);
	}

	if (dirtyFlags_[ColorInstances]) {
		vao_->SetBuffer(ColorInstances, GL_ARRAY_BUFFER, colorInstances_.count * sizeof(Vector4), nullptr, GL_STREAM_DRAW);
		vao_->SetVertexDataSource(ColorInstances, VertexAttribInstanceColor, 4, GL_FLOAT, false, 0, 0, colorInstances_.divisor);
	}

	if (dirtyFlags_[GeometryInstances]) {
		vao_->SetBuffer(GeometryInstances, GL_ARRAY_BUFFER, geometryInstances_.count * sizeof(Vector4), nullptr, GL_STREAM_DRAW);
		vao_->SetVertexDataSource(GeometryInstances, VertexAttribInstanceGeometry, 4, GL_FLOAT, false, 0, 0, geometryInstances_.divisor);
	}

	vao_->Unbind();

	dirtyFlags_.reset();
	modified.raise();
}

void GeometryInternal::ApplyInstanceBuffers() {
	for (int i = 0; i < SUEDE_COUNTOF(instanceBuffers_); ++i) {
		auto& ib = instanceBuffers_[i];
		if (ib.data) {
			vao_->UpdateBuffer(_InstanceBufferBegin + i, 0, ib.size, ib.data.get());
			ib.data = nullptr;
			ib.size = 0;
		}
	}
}
