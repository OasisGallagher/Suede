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

struct ShaderResource {
	std::string name;
	std::string path;
};

struct TextureResource {
	std::string name;
	std::string path;
};

class SUEDE_API Resources : public Singleton<Resources> {
public:
	void Import();

public:
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

public:
	Resources();

private:
	// TODO: ????
	const char* GetRelativePath(const char* path);

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

	typedef std::vector<TextureResource> TextureResourceContainer;
	TextureResourceContainer textureResources_;

	Mesh primitives_[PrimitiveTypeCount];
	Texture2D blackTexture_, whiteTexture_;
};
