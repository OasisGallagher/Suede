#pragma once
#include <map>
#include <vector>

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

class SUEDE_API Resources : public Singleton<Resources> {
	friend class Singleton<Resources>;

public:
	void Import();

public:
	Texture2D GetBlackTexture() { return blackTexture_; }
	Texture2D GetWhiteTexture() { return whiteTexture_; }

	std::string GetRootDirectory() { return "resources/"; }
	std::string GetModelDirectory() { return GetRootDirectory() + "models/"; }
	std::string GetShaderDirectory() { return GetRootDirectory() + "shaders/"; }
	std::string GetTextureDirectory() { return GetRootDirectory() + "textures/"; }

	Mesh GetPrimitive(PrimitiveType type) { return primitives_[type]; }
	Mesh CreatePrimitive(PrimitiveType type, float scale);
	Mesh CreateInstancedPrimitive(PrimitiveType type, float scale, const InstanceAttribute& color, const InstanceAttribute& geometry);

	void GetPrimitiveAttribute(MeshAttribute& attribute, PrimitiveType type, float scale);

	Shader FindShader(const std::string& path);
	Texture FindTexture(const std::string& path);
	Material FindMaterial(const std::string& name);

private:
	Resources() { ImportBuiltinResources(); }

private:
	void ImportShaderResources();
	void ImportTextureResources();
	void ImportBuiltinResources();

	void GetQuadMeshAttribute(MeshAttribute& attribute, float scale);
	void GetCubeMeshAttribute(MeshAttribute& attribute, float scale);
	Mesh CreateMesh(MeshAttribute &attribute);
	Texture2D CreateSolidTexture(uint color);

private:
	typedef std::map<std::string, Shader> ShaderContainer;
	ShaderContainer shaders_;

	typedef std::map<std::string, Texture> TextureContainer;
	TextureContainer textures_;

	typedef std::map<std::string, Material> MaterialContainer;
	MaterialContainer materials_;

	Mesh primitives_[PrimitiveTypeCount];
	Texture2D blackTexture_, whiteTexture_;
};
