#pragma once
#include <QDockWidget>

#include <glm/glm.hpp>

#include "world.h"
#include "entity.h"
#include "camera.h"
#include "projector.h"
#include "childwindow.h"

class QLineEdit;
class QPushButton;
class QListWidget;
class QColorDialog;

class Inspector : public QDockWidget, public ChildWindow, public WorldEventListener {
	Q_OBJECT

public:
	static Inspector* get();

public:
	Inspector(QWidget* parent);
	~Inspector();

public:
	virtual void init(Ui::Suede* ui);
	virtual void OnWorldEvent(WorldEventBasePointer e);

private slots:
	void onNameChanged();
	void onTextChanged();

	void onEditProperty();
	void onClickProperty();

	void onSliderValueChanged(int value);

	void onColorChanged(const QColor& color);

	void onSelectColor3(QWidget* widget, uint materialIndex, const QString& name);
	void onSelectColor4(QWidget* widget, uint materialIndex, const QString& name);
	void onSelectTexture(QWidget* widget, uint materialIndex, const QString& name);

	void onTagChanged(int index);
	void onActiveChanged(int state);
	void onSelectionChanged(const QList<Entity>& selected, const QList<Entity>& deselected);
	void onTransformChanged();
	void onResetButtonClicked();

private:
	void initTransformUI();

	void showView(bool show);

	void redraw();
	void drawTags();
	void drawTransform();

	void drawCamera(Camera camera);
	void drawProjector(Projector projector);

	void drawMesh();
	void drawTextMesh(Mesh mesh);
	void drawSubMeshes(Mesh mesh);

	void drawRenderer();
	void drawMaterial(Renderer renderer, uint materialIndex, const QStringList& shaders);
	void drawMaterialProperties(QWidgetList& widgets, Material material, uint materialIndex);

	void shrinkToFit(QListWidget* w);

	void onEntityTransformChanged(Entity target, uint prs);

	glm::vec3 readTransformFields(QLineEdit* x, QLineEdit* y, QLineEdit* z);
	void writeTransformFields(QLineEdit* x, QLineEdit* y, QLineEdit* z, const glm::vec3& v3);
	QString float2QString(float f);

	bool isPropertyVisible(const QString& name);

	QWidget* drawIntField(uint index, const QString& name, int value);
	QWidget* drawFloatField(uint index, const QString& name, float value);
	QWidget* drawTextureField(uint index, const QString& name, Texture value);
	QWidget* drawVec3Field(uint index, const QString& name, const glm::vec3& value);
	
	QWidget* drawColor3Field(uint index, const QString& name, const glm::vec3& value);
	QWidget* drawColor4Field(uint index, const QString& name, const glm::vec4& value);
	QWidget* drawColorField(uint index, const QString& name, VariantType type, const void* value);

	QWidget* drawVec4Field(uint index, const QString& name, const glm::vec4& value);

private:
	Entity target_;
	QList<QWidget*> groups_;
	QColorDialog* colorPicker_;
};
