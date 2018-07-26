#include "resources.h"
#include "tools/math2.h"
#include "os/filesystem.h"

void Resources::Import() {
	ImportShaderResources();
	ImportTextureResources();
}

Mesh Resources::CreatePrimitive(PrimitiveType type, float scale) {
	MeshAttribute attribute;
	GetPrimitiveAttribute(attribute, type, scale);

	return CreateMesh(attribute);
}

Mesh Resources::CreateInstancedPrimitive(PrimitiveType type, float scale, const InstanceAttribute& color, const InstanceAttribute& geometry) {
	MeshAttribute attribute;
	GetPrimitiveAttribute(attribute, type, scale);
	attribute.color = color;
	attribute.geometry = geometry;

	return CreateMesh(attribute);
}

void Resources::GetPrimitiveAttribute(MeshAttribute& attribute, PrimitiveType type, float scale) {
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
	attribute.topology = MeshTopology::TriangleStripe;

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
	attribute.topology = MeshTopology::Triangles;

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
		vertices[i] *= scale;
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

 	glm::vec3 normals[] = {
 		glm::vec3(0.333333f, 0.666667f, -0.666667f),
 		glm::vec3(-0.816497f, 0.408248f, -0.408248f),
 		glm::vec3(-0.333333f, 0.666667f, 0.666667f),
 		glm::vec3(0.816497f, 0.408248f, 0.408248f),
 		glm::vec3(0.666667f, -0.666667f, -0.333333f),
 		glm::vec3(-0.408248f, -0.408248f, -0.816497f),
 		glm::vec3(-0.666667f, -0.666667f, 0.333333f),
 		glm::vec3(0.408248f, -0.408248f, 0.816497f)
 	};
 	attribute.normals.assign(normals, normals + CountOf(normals));

	glm::vec2 texCoords[] = {
		glm::vec2(0.f, 1.f),
		glm::vec2(1.f, 1.f),
		glm::vec2(0.f, 1.f),
		glm::vec2(1.f, 1.f),
		glm::vec2(0.f, 0.f),
		glm::vec2(1.f, 0.f),
		glm::vec2(0.f, 0.f),
		glm::vec2(1.f, 0.f),
	};
	attribute.texCoords.assign(texCoords, texCoords + CountOf(texCoords));
}

Mesh Resources::CreateMesh(MeshAttribute &attribute) {
	Mesh mesh = NewMesh();
	mesh->SetAttribute(attribute);

	SubMesh subMesh = NewSubMesh();
	TriangleBias base{ attribute.indexes.size() };
	subMesh->SetTriangleBias(base);

	mesh->AddSubMesh(subMesh);
	return mesh;
}

Texture2D Resources::CreateSolidTexture(uint color) {
	Texture2D texture = NewTexture2D();
	texture->Load(TextureFormatRgba, &color, ColorStreamFormatRgba, 1, 1, 4);
	return texture;
}

void Resources::ImportBuiltinResources() {
	whiteTexture_ = CreateSolidTexture(0xffffffff);
	blackTexture_ = CreateSolidTexture(0xff000000);

	for (int type = PrimitiveTypeQuad; type < PrimitiveTypeCount; ++type) {
		primitives_[type] = CreatePrimitive((PrimitiveType)type, 1);
	}
}

void Resources::ImportShaderResources() {
}

void Resources::ImportTextureResources() {
}
