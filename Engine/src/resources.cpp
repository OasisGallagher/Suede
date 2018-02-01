#include <map>
#include "resources.h"
#include "tools/math2.h"
#include "os/filesystem.h"

typedef std::map<std::string, Shader> ShaderContainer;
static ShaderContainer shaders_;

typedef std::map<std::string, Texture> TextureContainer;
static TextureContainer textures_;

typedef std::map<std::string, Material> MaterialContainer;
static MaterialContainer materials_;

typedef std::vector<ShaderResource> ShaderResourceContainer;
static ShaderResourceContainer shaderResources_;

static MeshRenderer meshRenderer_;
static Mesh primitives_[PrimitiveTypeCount];
static Texture2D blackTexture_, whiteTexture_;

void Resources::Import() {
	if (!meshRenderer_) {
		meshRenderer_ = NewMeshRenderer();
		meshRenderer_->AddMaterial(nullptr);
	}

	ImportShaderResources();

}

MeshRenderer Resources::GetMeshRenderer() {
	return meshRenderer_;
}

Texture2D Resources::GetBlackTexture() {
	if (!blackTexture_) {
		blackTexture_ = CreateSolidTexture(0xff000000);
	}

	return blackTexture_;
}

Texture2D Resources::GetWhiteTexture() {
	if (!blackTexture_) {
		blackTexture_ = CreateSolidTexture(0xffffffff);
	}

	return blackTexture_;
}

const std::vector<ShaderResource>& Resources::GetShaderResources() {
	return shaderResources_;
}

std::string Resources::GetRootDirectory() {
	return "resources/";
}

Mesh Resources::GetPrimitive(PrimitiveType type) {
	if (primitives_[type]) {
		return primitives_[type];
	}

	primitives_[type] = CreatePrimitive(type, 1);
	return primitives_[type];
}

Mesh Resources::CreatePrimitive(PrimitiveType type, float scale) {
	MeshAttribute attribute;
	GetPrimitiveAttribute(type, scale, attribute);

	return CreateMesh(attribute);
}

Mesh Resources::CreateInstancedPrimitive(PrimitiveType type, float scale, const InstanceAttribute& color, const InstanceAttribute& geometry) {
	MeshAttribute attribute;
	GetPrimitiveAttribute(type, scale, attribute);
	attribute.color = color;
	attribute.geometry = geometry;

	return CreateMesh(attribute);
}

void Resources::GetPrimitiveAttribute(PrimitiveType type, float scale, MeshAttribute& attribute) {
	switch (type) {
	case PrimitiveTypeQuad:
		GetQuadMeshAttribute(attribute, scale);
		break;
	case PrimitiveTypeCube:
		break;
	}
}

Shader Resources::FindShader(const std::string& path) {
	ShaderContainer::iterator ite = shaders_.find(path);
	if (ite != shaders_.end()) {
		return ite->second;
	}

	Shader shader = NewShader();
	shaders_.insert(std::make_pair(path, shader));
	if (shader->Load(path)) {
		return shader;
	}

	return nullptr;
}

Texture Resources::FindTexture(const std::string& path) {
	return nullptr;
}

Material Resources::FindMaterial(const std::string& name) {
	MaterialContainer::iterator ite = materials_.find(name);
	if (ite != materials_.end()) {
		return ite->second;
	}

	return nullptr;
}

void Resources::GetQuadMeshAttribute(MeshAttribute& attribute, float scale) {
	attribute.topology = MeshTopologyTriangleStripes;

	glm::vec3 vertices[] = {
		glm::vec3(-0.5f * scale, -0.5f * scale, 0.f),
		glm::vec3(0.5f * scale, -0.5f * scale, 0.f),
		glm::vec3(-0.5f * scale,  0.5f * scale, 0.f),
		glm::vec3(0.5f * scale,  0.5f * scale, 0.f),
	};
	attribute.positions.assign(vertices, vertices + CountOf(vertices));

	glm::vec2 texCoords[] = {
		glm::vec2(0.f, 0.f),
		glm::vec2(1.f, 0.f),
		glm::vec2(0.f, 1.f),
		glm::vec2(1.f, 1.f),
	};
	attribute.texCoords.assign(texCoords, texCoords + CountOf(texCoords));

	attribute.normals.resize(CountOf(vertices), glm::vec3(0, 0, -1));

	int indexes[] = { 0, 1, 2, 3 };
	attribute.indexes.assign(indexes, indexes + CountOf(indexes));

	attribute.color.count = attribute.geometry.count = 0;
}

void Resources::GetCubeMeshAttribute(MeshAttribute& attribute, float scale) {
	throw "TODO";
}

Mesh Resources::CreateMesh(MeshAttribute &attribute) {
	Mesh mesh = NewMesh();
	mesh->SetAttribute(attribute);

	SubMesh subMesh = NewSubMesh();
	subMesh->SetTriangles(attribute.indexes.size(), 0, 0);

	mesh->AddSubMesh(subMesh);
	return mesh;
}

Texture2D Resources::CreateSolidTexture(uint color) {
	Texture2D texture = NewTexture2D();
	texture->Load(TextureFormatRgba, &color, ColorStreamFormatRgba, 1, 1);
	return texture;
}

void Resources::ImportShaderResources() {
	std::vector<std::string> paths;
	const char* reg = ".*\\.shader";
	FileSystem::ListAllFiles(paths, "resources/shaders", reg);
	FileSystem::ListAllFiles(paths, "resources/buildin/shaders", reg);

	shaderResources_.clear();
	// TODO: shader name.
	for (int i = 0; i < paths.size(); ++i) {
		ShaderResource sr = {
			FileSystem::GetFileNameWithoutExtension(paths[i]), paths[i]
		};

		shaderResources_.push_back(sr);
	}
}
