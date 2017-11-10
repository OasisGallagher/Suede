#pragma once
#include <map>

#include "shader.h"
#include "material.h"

class Resources {
public:
	static void Initialize();

public:
	static Shader FindShader(const std::string& path);
	static Texture FindTexture(const std::string& path);
	static Material FindMaterial(const std::string& name);
};
