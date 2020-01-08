#pragma once
#include "gui.h"
#include "material.h"
#include "os/filesystem.h"

class MaterialEditor {
public:
	static void draw(Material* material);

private:
	static void drawShaderSelector(Material* material);
	static void replaceShader(Material* material, const std::string& fullShaderPath);

	static void drawProperties(Material* material);
	static void drawBoolProperty(Material* material, const Property* p);
	static void drawRangedIntProperty(Material* material, const Property* p);
	static void drawRangedFloatProperty(Material* material, const Property* p);
	static void drawTextureProperty(Material* material, const Property* p);
	static void drawColorProperty(Material* material, const Property* p);
	static void drawFloatProperty(Material* material, const Property* p);
	static void drawVector3Property(Material* material, const Property* p);
	static void drawVector4Property(Material* material, const Property* p);

	static void drawTexture2DSelector(const Property* p, Texture2D* texture2D);
};
