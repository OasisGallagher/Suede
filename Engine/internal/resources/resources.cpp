#include "resources.h"
#include "internal/memory/factory.h"
#include "internal/base/glsldefines.h"
#include "internal/base/shaderinternal.h"
#include "internal/base/rendererinternal.h"

typedef std::map<std::string, Shader> ShaderContainer;
static ShaderContainer shaders;

typedef std::map<std::string, Renderer> RendererContainer;
static RendererContainer renderers;

Shader Resources::FindShader(const std::string& path) {
	ShaderContainer::iterator ite = shaders.find(path);
	if (ite != shaders.end()) {
		return ite->second;
	}

	Shader shader = CREATE_OBJECT(Shader);
	shaders.insert(std::make_pair(path, shader));
	if (shader->Load(path + GLSL_POSTFIX)) {
		return shader;
	}

	return nullptr;
}
