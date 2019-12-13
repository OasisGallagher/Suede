#pragma once
#include "mesh.h"
#include "shader.h"
#include "material.h"
#include "renderer.h"

#include "tools/enum.h"
#include "tools/singleton.h"

BETTER_ENUM(PrimitiveType, int,
	Quad,
	Cube
)

class SUEDE_API Resources : private Singleton2<Resources> {
	friend class Singleton<Resources>;
	SUEDE_DECLARE_IMPLEMENTATION(Resources)

public:
	static void Import();

	static Texture2D* GetBlackTexture();
	static Texture2D* GetWhiteTexture();

	static std::string GetRootDirectory();
	static std::string GetModelDirectory();
	static std::string GetShaderDirectory();
	static std::string GetTextureDirectory();

	static Mesh* GetPrimitive(PrimitiveType type);
	static ref_ptr<Mesh> CreatePrimitive(PrimitiveType type, float scale);
	static ref_ptr<Mesh> CreateInstancedPrimitive(PrimitiveType type, float scale, const InstanceAttribute& color, const InstanceAttribute& geometry);

	static void GetPrimitiveAttribute(MeshAttribute& attribute, PrimitiveType type, float scale);

	static Shader* FindShader(const std::string& path);
	static Texture* FindTexture(const std::string& path);
	static Material* FindMaterial(const std::string& name);

private:
	Resources();
};
