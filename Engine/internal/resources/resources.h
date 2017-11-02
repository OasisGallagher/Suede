#pragma once
#include <map>

#include "shader.h"

class Resources {
public:
	static Shader FindShader(const std::string& path);
};
