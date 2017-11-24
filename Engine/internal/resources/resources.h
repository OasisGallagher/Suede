#pragma once
#include "mesh.h"
#include "shader.h"
#include "material.h"

enum PrimitiveType {
	PrimitiveTypeQuad,
	PrimitiveTypeCube,
	PrimitiveTypeCount,
};

class Resources {
public:
	static void Initialize();

public:
	static Mesh GetPrimitive(PrimitiveType type);
	static Mesh CreatePrimitive(PrimitiveType type);
	static Mesh CreateInstancedPrimitive(PrimitiveType type, const InstanceAttribute& color, const InstanceAttribute& geometry);

	static void GetPrimitiveAttribute(PrimitiveType type, MeshAttribute& attribute);

	static Shader FindShader(const std::string& path);
	static Texture FindTexture(const std::string& path);
	static Material FindMaterial(const std::string& name);

private:
	static void GetQuadMeshAttribute(MeshAttribute& attribute);
	static void GetCubeMeshAttribute(MeshAttribute& attribute);
	static Mesh CreateMesh(MeshAttribute &attribute);
};
