#pragma once
#include "shader.h"
#include "surface.h"
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
	static Surface GetPrimitive(PrimitiveType type);
	static Surface CreatePrimitive(PrimitiveType type);
	static Surface CreateInstancedPrimitive(PrimitiveType type, const InstanceAttribute& color, const InstanceAttribute& geometry);

	static void GetPrimitiveAttribute(PrimitiveType type, SurfaceAttribute& attribute);

	static Shader FindShader(const std::string& path);
	static Texture FindTexture(const std::string& path);
	static Material FindMaterial(const std::string& name);

private:
	static void GetQuadSurfaceAttribute(SurfaceAttribute& attribute);
	static void GetCubeSurfaceAttribute(SurfaceAttribute& attribute);
	static Surface CreateSurface(SurfaceAttribute &attribute);
};
