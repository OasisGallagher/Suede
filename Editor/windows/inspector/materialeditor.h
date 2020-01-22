#pragma once
#include <QWidget>

#include "gui.h"
#include "material.h"
#include "os/filesystem.h"

class MaterialEditor {
	struct Modification {
		Shader* shader;
		QList<Property> properties;

		bool empty() const;
		void apply(Material* material);
	};

public:
	void draw(Material* material, QWidget* parent);
	Modification constantDraw(Material* material, QWidget* parent);

private:
	Shader* drawShaderSelector(Material* material, QWidget* parent);

	void drawProperties(QList<Property>& changed, Material* material);
	void drawBoolProperty(QList<Property>& changed, Material* material, const Property* p);
	void drawRangedIntProperty(QList<Property>& changed, Material* material, const Property* p);
	void drawRangedFloatProperty(QList<Property>& changed, Material* material, const Property* p);
	void drawTextureProperty(QList<Property>& changed, Material* material, const Property* p);
	void drawColorProperty(QList<Property>& changed, Material* material, const Property* p);
	void drawFloatProperty(QList<Property>& changed, Material* material, const Property* p);
	void drawVector3Property(QList<Property>& changed, Material* material, const Property* p);
	void drawVector4Property(QList<Property>& changed, Material* material, const Property* p);
};
