#include <glm/glm.hpp>
#include <QColorDialog>

#include "renderer.h"
#include "custominspector.h"

class RendererInspector : public CustomInspector {
	Q_OBJECT

public:
	RendererInspector(Object object);

private:
	void drawMaterial(Renderer renderer, uint materialIndex, QLayout* materialsLayout);
	QGroupBox* createMaterialBox(Renderer renderer, uint materialIndex, QLayout* materialsLayout);

	bool isPropertyVisible(const QString& name);
	bool updateMaterial(uint materialIndex, const QString& shaderPath);
	void drawMaterialProperties(QWidgetList& widgets, Material material, uint materialIndex);

	QWidget* drawIntField(uint materialIndex, const QString& name, int value);
	QWidget* drawFloatField(uint materialIndex, const QString& name, float value);
	QWidget* drawTextureField(uint materialIndex, const QString& name, Texture value);
	QWidget* drawColorField(uint materialIndex, const QString& name, VariantType type, const void* value);
	QWidget* drawColor3Field(uint materialIndex, const QString& name, const glm::vec3& value);
	QWidget* drawColor4Field(uint materialIndex, const QString& name, const glm::vec4& value);
	QWidget* drawVec3Field(uint materialIndex, const QString& name, const glm::vec3& value);
	QWidget* drawVec4Field(uint materialIndex, const QString& name, const glm::vec4& value);

private slots:
	void onEditProperty();
	void onColorPicked(const QColor& color);
	void onShaderSelectionChanged(const QString& path);

	void onSelectTexture(QWidget* widget, uint materialIndex, const QString& name);
	void onSelectColor3(QWidget* widget, uint materialIndex, const QString& name);
	void onSelectColor4(QWidget* widget, uint materialIndex, const QString& name);
};
