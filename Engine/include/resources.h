#pragma once
#include "mesh.h"
#include "shader.h"
#include "material.h"
#include "renderer.h"
#include "tools/singleton.h"

enum PrimitiveType {
	PrimitiveTypeQuad,
	PrimitiveTypeCube,
	PrimitiveTypeCount,
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

	virtual Mesh GetPrimitive(PrimitiveType type) = 0;
	virtual Mesh CreatePrimitive(PrimitiveType type, float scale) = 0;
	virtual Mesh CreateInstancedPrimitive(PrimitiveType type, float scale, const InstanceAttribute& color, const InstanceAttribute& geometry) = 0;

	virtual void GetPrimitiveAttribute(MeshAttribute& attribute, PrimitiveType type, float scale) = 0;

	virtual Shader FindShader(const std::string& path) = 0;
	virtual Texture FindTexture(const std::string& path) = 0;
	virtual Material FindMaterial(const std::string& name) = 0;
};
