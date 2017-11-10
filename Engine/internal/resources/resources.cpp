#include "resources.h"
#include "internal/memory/factory.h"
#include "internal/base/glsldefines.h"
#include "internal/base/shaderinternal.h"
#include "internal/base/rendererinternal.h"

typedef std::map<std::string, Shader> ShaderContainer;
static ShaderContainer shaders;

typedef std::map<std::string, Texture> TextureContainer;
static TextureContainer textures;

typedef std::map<std::string, Material> MaterialContainer;
static MaterialContainer materials;

void Resources::Initialize() {

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
