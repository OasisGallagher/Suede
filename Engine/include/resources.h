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
public:
	virtual void Import() = 0;

	virtual Texture2D GetBlackTexture() = 0;
	virtual Texture2D GetWhiteTexture() = 0;

	virtual std::string GetRootDirectory() = 0;
	virtual std::string GetModelDirectory() = 0;
	virtual std::string GetShaderDirectory() = 0;
	virtual std::string GetTextureDirectory() = 0;

	virtual void CreatePrimitive(Mesh mesh, PrimitiveType type, float scale = 1) = 0;
	virtual void CreateInstancedPrimitive(Mesh mesh, PrimitiveType type, float scale, const InstanceAttribute& color, const InstanceAttribute& geometry) = 0;

	virtual void GetPrimitiveAttribute(MeshAttribute& attribute, PrimitiveType type, float scale) = 0;

	virtual Shader FindShader(const std::string& path) = 0;
	virtual Texture FindTexture(const std::string& path) = 0;
	virtual Material FindMaterial(const std::string& name) = 0;
};
