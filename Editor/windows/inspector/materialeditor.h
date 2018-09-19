#pragma once
#include "gui.h"
#include "material.h"

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

	static void drawProperties(Material material);
	static void drawTextureProperty(Material material, const Property* p);
	static void drawColorProperty(Material material, const Property* p);
	static void drawFloatProperty(Material material, const Property* p);
	static void drawVector3Property(Material material, const Property* p);
	static void drawVector4Property(Material material, const Property* p);

private:
	static QVector<MainContextCommand*> commands_;
};
