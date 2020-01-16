#pragma once
#include <QWidget>

#include "gui.h"
#include "material.h"
#include "os/filesystem.h"

class MaterialEditor {
public:
	void draw(Material* material, QWidget* parent);

private:
	void drawShaderSelector(Material* material, QWidget* parent);
	void replaceShader(Material* material, const std::string& fullShaderPath);

	void drawProperties(Material* material);
	void drawBoolProperty(Material* material, const Property* p);
	void drawRangedIntProperty(Material* material, const Property* p);
	void drawRangedFloatProperty(Material* material, const Property* p);
	void drawTextureProperty(Material* material, const Property* p);
	void drawColorProperty(Material* material, const Property* p);
	void drawFloatProperty(Material* material, const Property* p);
	void drawVector3Property(Material* material, const Property* p);
	void drawVector4Property(Material* material, const Property* p);

	void drawTexture2DSelector(const Property* p, Texture2D* texture2D);
};
