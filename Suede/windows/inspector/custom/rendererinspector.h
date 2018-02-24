#include <glm/glm.hpp>
#include <QColorDialog>

#include "renderer.h"
#include "custominspector.h"

class RendererInspector : public CustomInspector {
	Q_OBJECT

public:
	RendererInspector(Object object);

private:
	void initializeColorPicker();

	void drawMaterial(Renderer renderer, uint materialIndex, const QStringList& shaders, QLayout* materialsLayout);
	void drawMaterialProperties(QWidgetList& widgets, Material material, uint materialIndex);
	bool isPropertyVisible(const QString& name);

	QWidget* drawIntField(uint index, const QString& name, int value);
	QWidget* drawFloatField(uint index, const QString& name, float value);
	QWidget* drawTextureField(uint index, const QString& name, Texture value);
	QWidget* drawColorField(uint index, const QString& name, VariantType type, const void* value);
	QWidget* drawColor3Field(uint index, const QString& name, const glm::vec3& value);
	QWidget* drawColor4Field(uint index, const QString& name, const glm::vec4& value);
	QWidget* drawVec3Field(uint index, const QString& name, const glm::vec3& value);
	QWidget* drawVec4Field(uint index, const QString& name, const glm::vec4& value);

private slots:
	void onEditProperty();
	void onColorChanged(const QColor& color);
	void onSelectTexture(QWidget* widget, uint materialIndex, const QString& name);
	void onSelectColor3(QWidget* widget, uint materialIndex, const QString& name);
	void onSelectColor4(QWidget* widget, uint materialIndex, const QString& name);

private:
	QColorDialog* colorPicker_;
};
