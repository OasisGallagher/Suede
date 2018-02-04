#pragma once
#include <vector>

#include "mesh.h"
#include "shader.h"
#include "material.h"
#include "renderer.h"

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

class SUEDE_API Resources {
public:
	static void Import();

public:
	static Texture2D GetBlackTexture();
	static Texture2D GetWhiteTexture();

	static const std::vector<ShaderResource>& GetShaderResources();

	static std::string GetRootDirectory();

	/** TODO: GL.End() instead ?
	 * @brief: Temporary mesh renderer.
	 */
	static MeshRenderer GetMeshRenderer();

	static Mesh GetPrimitive(PrimitiveType type);
	static Mesh CreatePrimitive(PrimitiveType type, float scale);
	static Mesh CreateInstancedPrimitive(PrimitiveType type, float scale, const InstanceAttribute& color, const InstanceAttribute& geometry);

	static void GetPrimitiveAttribute(PrimitiveType type, float scale, MeshAttribute& attribute);

	static Shader FindShader(const std::string& path);
	static Texture FindTexture(const std::string& path);
	static Material FindMaterial(const std::string& name);

private:
	static const char* GetRelativePath(const char* path);

	static void ImportShaderResources();
	static void ImportTextureResources();

	static void GetQuadMeshAttribute(MeshAttribute& attribute, float scale);
	static void GetCubeMeshAttribute(MeshAttribute& attribute, float scale);
	static Mesh CreateMesh(MeshAttribute &attribute);
	static Texture2D CreateSolidTexture(uint color);

private:
	Resources();
};
