#pragma once
#include "mesh.h"
#include "shader.h"
#include "material.h"

enum PrimitiveType {
	PrimitiveTypeQuad,
	PrimitiveTypeCube,
	PrimitiveTypeCount,
};

class SUEDE_API Resources {
public:
	static void Import();

public:
	static Texture2D GetBlackTexture();
	static Texture2D GetWhiteTexture();

	static Mesh GetPrimitive(PrimitiveType type);
	static Mesh CreatePrimitive(PrimitiveType type, float scale);
	static Mesh CreateInstancedPrimitive(PrimitiveType type, float scale, const InstanceAttribute& color, const InstanceAttribute& geometry);

	static void GetPrimitiveAttribute(PrimitiveType type, float scale, MeshAttribute& attribute);

	static Shader FindShader(const std::string& path);
	static Texture FindTexture(const std::string& path);
	static Material FindMaterial(const std::string& name);

private:
	static void GetQuadMeshAttribute(MeshAttribute& attribute, float scale);
	static void GetCubeMeshAttribute(MeshAttribute& attribute, float scale);
	static Mesh CreateMesh(MeshAttribute &attribute);
	static Texture2D CreateSolidTexture(uint color);

private:
	Resources();
};
