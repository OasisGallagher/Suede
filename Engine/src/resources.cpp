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

typedef std::vector<TextureResource> TextureResourceContainer;
static TextureResourceContainer textureResources_;

static MeshRenderer meshRenderer_;
static Mesh primitives_[PrimitiveTypeCount];
static Texture2D blackTexture_, whiteTexture_;

void Resources::Import() {
	if (!meshRenderer_) {
		meshRenderer_ = NewMeshRenderer();
		meshRenderer_->AddMaterial(nullptr);
	}

	ImportShaderResources();
	ImportTextureResources();
}

MeshRenderer Resources::GetAuxMeshRenderer() {
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
		GetCubeMeshAttribute(attribute, scale);
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
}

void Resources::GetCubeMeshAttribute(MeshAttribute& attribute, float scale) {
	attribute.topology = MeshTopologyTriangles;

	glm::vec3 vertices[] = {
		glm::vec3(0.5f, 0.5f, -0.5f),
		glm::vec3(-0.5f, 0.5f, -0.5f),
		glm::vec3(-0.5f, 0.5f, 0.5f),
		glm::vec3(0.5f, 0.5f, 0.5f),
		glm::vec3(0.5f, -0.5f, -0.5f),
		glm::vec3(-0.5f, -0.5f, -0.5f),
		glm::vec3(-0.5f, -0.5f, 0.5f),
		glm::vec3(0.5f, -0.5f, 0.5f),
	};
	for (int i = 0; i < CountOf(vertices); ++i) {
		vertices[i] *= 2;
	}

	attribute.positions.assign(vertices, vertices + CountOf(vertices));

	uint indexes[] = {
		0, 1, 2,
		0, 2, 3,
		0, 4, 5,
		0, 5, 1,
		1, 5, 6,
		1, 6, 2,
		2, 6, 7,
		2, 7, 3,
		3, 7, 4,
		3, 4, 0,
		4, 7, 6,
		4, 6, 5
	};
	attribute.indexes.assign(indexes, indexes + CountOf(indexes));

// 	glm::vec3 normals[] = {
// 		glm::vec3(0.333333f, 0.666667f, -0.666667f),
// 		glm::vec3(-0.816497f, 0.408248f, -0.408248f),
// 		glm::vec3(-0.333333f, 0.666667f, 0.666667f),
// 		glm::vec3(0.816497f, 0.408248f, 0.408248f),
// 		glm::vec3(0.666667f, -0.666667f, -0.333333f),
// 		glm::vec3(-0.408248f, -0.408248f, -0.816497f),
// 		glm::vec3(-0.666667f, -0.666667f, 0.333333f),
// 		glm::vec3(0.408248f, -0.408248f, 0.816497f)
// 	};
// 	attribute.normals.assign(normals, normals + CountOf(normals));

// 	glm::vec2 texCoords[] = {
// 		glm::vec2(0.f, 1.f),
// 		glm::vec2(1.f, 1.f),
// 		glm::vec2(0.f, 1.f),
// 		glm::vec2(1.f, 1.f),
// 		glm::vec2(0.f, 0.f),
// 		glm::vec2(1.f, 0.f),
// 		glm::vec2(0.f, 0.f),
// 		glm::vec2(1.f, 0.f),
// 	};
// 	attribute.texCoords.assign(texCoords, texCoords + CountOf(texCoords));
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
			FileSystem::GetFileNameWithoutExtension(paths[i]), 
			GetRelativePath(paths[i].c_str())
		};

		shaderResources_.push_back(sr);
	}
}

void Resources::ImportTextureResources() {
	std::vector<std::string> paths;
	const char* reg = ".*\\.(jpg|png|tif|bmp|tga|dds)";
	FileSystem::ListAllFiles(paths, "resources/textures", reg);

	textureResources_.clear();
	for (int i = 0; i < paths.size(); ++i) {
		TextureResource tr = {
			FileSystem::GetFileNameWithoutExtension(paths[i]), 
			GetRelativePath(paths[i].c_str())
		};

		textureResources_.push_back(tr);
	}
}

const char* Resources::GetRelativePath(const char* path) {
	for (; *path != 0 && *path != '/' && *path != '\\'; ++path) {
	}

	if (*path != 0) { ++path; }
	return path;
}
