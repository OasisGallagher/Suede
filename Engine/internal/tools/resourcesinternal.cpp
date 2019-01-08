#include "resourcesinternal.h"

#include "tools/math2.h"
#include "os/filesystem.h"
#include "memory/memory.h"
#include "geometryutility.h"

Resources::Resources() : singleton2<Resources>(MEMORY_NEW(ResourcesInternal), Memory::DeleteRaw<ResourcesInternal>) {}
void Resources::Import() { _suede_dinstance()->Import(); }
Texture2D Resources::GetBlackTexture() { return _suede_dinstance()->GetBlackTexture(); }
Texture2D Resources::GetWhiteTexture() { return _suede_dinstance()->GetWhiteTexture(); }
std::string Resources::GetRootDirectory() { return _suede_dinstance()->GetRootDirectory(); }
std::string Resources::GetModelDirectory() { return _suede_dinstance()->GetModelDirectory(); }
std::string Resources::GetShaderDirectory() { return _suede_dinstance()->GetShaderDirectory(); }
std::string Resources::GetTextureDirectory() { return _suede_dinstance()->GetTextureDirectory(); }
Mesh Resources::GetPrimitive(PrimitiveType type) { return _suede_dinstance()->GetPrimitive(type); }
Mesh Resources::CreatePrimitive(PrimitiveType type, float scale) { return _suede_dinstance()->CreatePrimitive(type, scale); }
Mesh Resources::CreateInstancedPrimitive(PrimitiveType type, float scale, const InstanceAttribute& color, const InstanceAttribute& geometry)
{ return _suede_dinstance()->CreateInstancedPrimitive(type, scale, color, geometry); }
void Resources::GetPrimitiveAttribute(MeshAttribute& attribute, PrimitiveType type, float scale) { _suede_dinstance()->GetPrimitiveAttribute(attribute, type, scale); }
Shader Resources::FindShader(const std::string& path) { return _suede_dinstance()->FindShader(path); }
Texture Resources::FindTexture(const std::string& path) { return _suede_dinstance()->FindTexture(path); }
Material Resources::FindMaterial(const std::string& name) { return _suede_dinstance()->FindMaterial(name); }

ResourcesInternal::ResourcesInternal() {
	ImportBuiltinResources();
	Import();
}

void ResourcesInternal::Import() {
	ImportShaderResources();
	ImportTextureResources();
}

Mesh ResourcesInternal::CreatePrimitive(PrimitiveType type, float scale) {
	MeshAttribute attribute;
	GetPrimitiveAttribute(attribute, type, scale);

	return InitializeMesh(attribute);
}

Mesh ResourcesInternal::CreateInstancedPrimitive(PrimitiveType type, float scale, const InstanceAttribute& color, const InstanceAttribute& geometry) {
	MeshAttribute attribute;
	GetPrimitiveAttribute(attribute, type, scale);
	attribute.color = color;
	attribute.geometry = geometry;

	return InitializeMesh(attribute);
}

void ResourcesInternal::GetPrimitiveAttribute(MeshAttribute& attribute, PrimitiveType type, float scale) {
	switch (type) {
		case PrimitiveType::Quad:
			GetQuadMeshAttribute(attribute, scale);
			break;
		case PrimitiveType::Cube:
			GetCubeMeshAttribute(attribute, scale);
			break;
	}
}

Shader ResourcesInternal::FindShader(const std::string& path) {
	ShaderContainer::iterator ite = shaders_.find(path);
	if (ite != shaders_.end()) {
		return ite->second;
	}

	Shader shader = new IShader();
	shaders_.insert(std::make_pair(path, shader));
	if (shader->Load(path)) {
		return shader;
	}

	return nullptr;
}

Texture ResourcesInternal::FindTexture(const std::string& path) {
	return nullptr;
}

Material ResourcesInternal::FindMaterial(const std::string& name) {
	MaterialContainer::iterator ite = materials_.find(name);
	if (ite != materials_.end()) {
		return ite->second;
	}

	return nullptr;
}

void ResourcesInternal::GetQuadMeshAttribute(MeshAttribute& attribute, float scale) {
	attribute.topology = MeshTopology::TriangleStripe;

	attribute.positions.assign({
		glm::vec3(-0.5f * scale, -0.5f * scale, 0.f),
		glm::vec3(0.5f * scale, -0.5f * scale, 0.f),
		glm::vec3(-0.5f * scale,  0.5f * scale, 0.f),
		glm::vec3(0.5f * scale,  0.5f * scale, 0.f),
	});

	attribute.texCoords[0].assign({
		glm::vec2(0.f, 0.f),
		glm::vec2(1.f, 0.f),
		glm::vec2(0.f, 1.f),
		glm::vec2(1.f, 1.f),
	});

	attribute.indexes.assign({ 0, 1, 2, 3 });
}

void ResourcesInternal::GetCubeMeshAttribute(MeshAttribute& attribute, float scale) {
	attribute.topology = MeshTopology::Triangles;
	GeometryUtility::GetCuboidCoordinates(attribute.positions, glm::vec3(0), glm::vec3(1), &attribute.indexes);

	for (int i = 0; i < attribute.positions.size(); ++i) {
		attribute.positions[i] *= scale;
	}

	attribute.normals.assign({
		glm::vec3(0.333333f, 0.666667f, -0.666667f),
		glm::vec3(-0.816497f, 0.408248f, -0.408248f),
		glm::vec3(-0.333333f, 0.666667f, 0.666667f),
		glm::vec3(0.816497f, 0.408248f, 0.408248f),
		glm::vec3(0.666667f, -0.666667f, -0.333333f),
		glm::vec3(-0.408248f, -0.408248f, -0.816497f),
		glm::vec3(-0.666667f, -0.666667f, 0.333333f),
		glm::vec3(0.408248f, -0.408248f, 0.816497f)
	});

	attribute.texCoords[0].assign({
		glm::vec2(0.f, 1.f),
		glm::vec2(1.f, 1.f),
		glm::vec2(0.f, 1.f),
		glm::vec2(1.f, 1.f),
		glm::vec2(0.f, 0.f),
		glm::vec2(1.f, 0.f),
		glm::vec2(0.f, 0.f),
		glm::vec2(1.f, 0.f),
	});
}

Mesh ResourcesInternal::InitializeMesh(MeshAttribute& attribute) {
	Mesh mesh = new IMesh();
	mesh->SetAttribute(attribute);

	SubMesh subMesh = new ISubMesh();
	TriangleBias base{ attribute.indexes.size() };
	subMesh->SetTriangleBias(base);

	mesh->AddSubMesh(subMesh);

	return mesh;
}

Texture2D ResourcesInternal::CreateSolidTexture(uint color) {
	Texture2D texture = new ITexture2D();
	texture->SetPixels(TextureFormat::Rgba, &color, ColorStreamFormat::Rgba, 1, 1, 4);
	return texture;
}

void ResourcesInternal::ImportBuiltinResources() {
	whiteTexture_ = CreateSolidTexture(0xffffffff);
	blackTexture_ = CreateSolidTexture(0xff000000);

	for (int type = PrimitiveType::Quad; type < PrimitiveType::size(); ++type) {
		primitives_[type] = CreatePrimitive((PrimitiveType)type, 1);
	}
}

void ResourcesInternal::ImportShaderResources() {
}

void ResourcesInternal::ImportTextureResources() {
}
