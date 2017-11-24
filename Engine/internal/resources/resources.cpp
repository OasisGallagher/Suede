#include <map>

#include "resources.h"
#include "tools/math2.h"
#include "internal/memory/factory.h"
#include "internal/base/glsldefines.h"
#include "internal/base/meshinternal.h"
#include "internal/base/shaderinternal.h"
#include "internal/base/rendererinternal.h"

typedef std::map<std::string, Shader> ShaderContainer;
static ShaderContainer shaders;

typedef std::map<std::string, Texture> TextureContainer;
static TextureContainer textures;

typedef std::map<std::string, Material> MaterialContainer;
static MaterialContainer materials;

static Mesh primitives[PrimitiveTypeCount];

void Resources::Initialize() {

}

Mesh Resources::GetPrimitive(PrimitiveType type) {
	if (primitives[type]) {
		return primitives[type];
	}

	primitives[type] = CreatePrimitive(type);
	return primitives[type];
}

Mesh Resources::CreatePrimitive(PrimitiveType type) {
	MeshAttribute attribute;
	GetPrimitiveAttribute(type, attribute);

	return CreateMesh(attribute);
}

Mesh Resources::CreateInstancedPrimitive(PrimitiveType type, const InstanceAttribute& color, const InstanceAttribute& geometry) {
	MeshAttribute attribute;
	GetPrimitiveAttribute(type, attribute);
	attribute.color = color;
	attribute.geometry = geometry;

	return CreateMesh(attribute);
}

void Resources::GetPrimitiveAttribute(PrimitiveType type, MeshAttribute& attribute) {
	switch (type) {
	case PrimitiveTypeQuad:
		GetQuadMeshAttribute(attribute);
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

	Shader shader = CREATE_OBJECT(Shader);
	shaders.insert(std::make_pair(path, shader));
	if (shader->Load(path)) {
		return shader;
	}

	return nullptr;
}

Texture Resources::FindTexture(const std::string & path) {
	return nullptr;
}

Material Resources::FindMaterial(const std::string& name) {
	MaterialContainer::iterator ite = materials.find(name);
	if (ite != materials.end()) {
		return ite->second;
	}

	return nullptr;
}

void Resources::GetQuadMeshAttribute(MeshAttribute& attribute) {
	glm::vec3 vertices[] = {
		glm::vec3(-1.f, -1.f, 0.f),
		glm::vec3(1.f, -1.f, 0.f),
		glm::vec3(-1.f,  1.f, 0.f),
		glm::vec3(1.f,  1.f, 0.f),
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

void Resources::GetCubeMeshAttribute(MeshAttribute& attribute) {

}

Mesh Resources::CreateMesh(MeshAttribute &attribute) {
	Mesh mesh = CREATE_OBJECT(Mesh);
	mesh->SetAttribute(attribute);
	mesh->SetTopology(MeshTopologyTriangleStripes);

	SubMesh subMesh = CREATE_OBJECT(SubMesh);
	subMesh->SetTriangles(attribute.indexes.size(), 0, 0);

	mesh->AddSubMesh(subMesh);
	return mesh;
}
