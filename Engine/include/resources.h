#pragma once
#include "mesh.h"
#include "shader.h"
#include "material.h"
#include "renderer.h"
#include "tools/singleton.h"

enum class PrimitiveType {
	Quad,
	Cube,

	_Count,
};

class SUEDE_API Resources : public Singleton2<Resources> {
	friend class Singleton<Resources>;
	SUEDE_DECLARE_IMPLEMENTATION(Resources)

public:
	void Import();

	Texture2D GetBlackTexture();
	Texture2D GetWhiteTexture();

	std::string GetRootDirectory();
	std::string GetModelDirectory();
	std::string GetShaderDirectory();
	std::string GetTextureDirectory();

	Mesh GetPrimitive(PrimitiveType type);
	Mesh CreatePrimitive(PrimitiveType type, float scale);
	Mesh CreateInstancedPrimitive(PrimitiveType type, float scale, const InstanceAttribute& color, const InstanceAttribute& geometry);

	void GetPrimitiveAttribute(MeshAttribute& attribute, PrimitiveType type, float scale);

	Shader FindShader(const std::string& path);
	Texture FindTexture(const std::string& path);
	Material FindMaterial(const std::string& name);

private:
	Resources();
};
