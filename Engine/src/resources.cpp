#include <map>
#include "mesh.h"
#include "math2.h"
#include "shader.h"
#include "material.h"
#include "resources.h"

typedef std::map<std::string, Shader> ShaderContainer;
static ShaderContainer shaders;

typedef std::map<std::string, Texture> TextureContainer;
static TextureContainer textures;

typedef std::map<std::string, Material> MaterialContainer;
static MaterialContainer materials;

Texture2D blackTexture, whiteTexture;

static Mesh primitives[PrimitiveTypeCount];

void Resources::Import() {

}

Texture2D Resources::GetBlackTexture() {
	if (!blackTexture) {
		blackTexture = CreateSolidTexture(0xff000000);
	}

	return blackTexture;
}

Texture2D Resources::GetWhiteTexture() {
	if (!blackTexture) {
		blackTexture = CreateSolidTexture(0xffffffff);
	}

	return blackTexture;
}

Mesh Resources::GetPrimitive(PrimitiveType type) {
	if (primitives[type]) {
		return primitives[type];
	}

	primitives[type] = CreatePrimitive(type, 1);
	return primitives[type];
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
	ShaderContainer::iterator ite = shaders.find(path);
	if (ite != shaders.end()) {
		return ite->second;
	}

	Shader shader = NewShader();
	shaders.insert(std::make_pair(path, shader));
	if (shader->Load(path)) {
		return shader;
	}

	return nullptr;
}

Texture Resources::FindTexture(const std::string& path) {
	return nullptr;
}

Material Resources::FindMaterial(const std::string& name) {
	MaterialContainer::iterator ite = materials.find(name);
	if (ite != materials.end()) {
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
	texture->Load(&color, ColorFormatRgba, 1, 1);
	return texture;
}
