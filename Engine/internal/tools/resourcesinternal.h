#pragma once
#include "resources.h"

#include <map>
#include <vector>

class ResourcesInternal : public Resources {
public:
	ResourcesInternal();

public:
	virtual void Import();

	virtual Texture2D GetBlackTexture() { return blackTexture_; }
	virtual Texture2D GetWhiteTexture() { return whiteTexture_; }

	virtual std::string GetRootDirectory() { return "resources/"; }
	virtual std::string GetModelDirectory() { return GetRootDirectory() + "models/"; }
	virtual std::string GetShaderDirectory() { return GetRootDirectory() + "shaders/"; }
	virtual std::string GetTextureDirectory() { return GetRootDirectory() + "textures/"; }

	virtual Mesh GetPrimitive(PrimitiveType type) { return primitives_[type]; }
	virtual Mesh CreatePrimitive(PrimitiveType type, float scale);
	virtual Mesh CreateInstancedPrimitive(PrimitiveType type, float scale, const InstanceAttribute& color, const InstanceAttribute& geometry);

	virtual void GetPrimitiveAttribute(MeshAttribute& attribute, PrimitiveType type, float scale);

	virtual Shader FindShader(const std::string& path);
	virtual Texture FindTexture(const std::string& path);
	virtual Material FindMaterial(const std::string& name);

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
