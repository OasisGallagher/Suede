#pragma once
#include "gui.h"
#include "material.h"
#include "os/filesystem.h"

#include <QVector>

class MainContextCommand {
public:
	virtual ~MainContextCommand() {}

public:
	virtual void run() = 0;
};

class MaterialEditor {
public:
	static void draw(Material material);
	static void runMainContextCommands();

private:
	static void drawShaderSelector(Material material);
	static void replaceShader(Material material, const std::string& fullShaderPath);

	static void drawProperties(Material material);
	typedef void(*DrawMethod)(Material material, const Property* p);
	static void drawBoolProperty(Material material, const Property* p);
	static void drawRangedIntProperty(Material material, const Property* p);
	static void drawRangedFloatProperty(Material material, const Property* p);
	static void drawTextureProperty(Material material, const Property* p);
	static void drawColorProperty(Material material, const Property* p);
	static void drawFloatProperty(Material material, const Property* p);
	static void drawVector3Property(Material material, const Property* p);
	static void drawVector4Property(Material material, const Property* p);

	static void drawTexture2DSelector(const Property* p, Texture2D texture2D);

private:
	static QVector<MainContextCommand*> commands_;
};
