#pragma once
#include <QDockWidget>

#include <glm/glm.hpp>

#include "world.h"
#include "sprite.h"
#include "childwindow.h"

class QLineEdit;
class QPushButton;
class QListWidget;

class Inspector : public QDockWidget, public ChildWindow, public WorldEventListener {
	Q_OBJECT

public:
	static Inspector* get();

public:
	Inspector(QWidget* parent);
	~Inspector();

public:
	virtual void init(Ui::Suede* ui);
	virtual void OnWorldEvent(const WorldEventBase* e);

private slots:
	void onNameChanged();
	void onTextChanged();

	void onEditProperty();
	void onClickProperty();

	void onTagChanged(int index);
	void onActiveChanged(int state);
	void onSelectionChanged(const QList<Sprite>& selected, const QList<Sprite>& deselected);
	void onTransformChanged();
	void onResetButtonClicked();

private:
	void initTransformUI();

	void showView(bool show);

	void reload();
	void reloadTags();
	void reloadTransform();

	void reloadMesh();
	void reloadRenderer();

	void shrinkListWidget(QListWidget* w);

	void onSpriteTransformChanged(SpriteTransformChangedEvent* e);

	glm::vec3 readTransformFields(QLineEdit* x, QLineEdit* y, QLineEdit* z);
	void writeTransformFields(QLineEdit* x, QLineEdit* y, QLineEdit* z, const glm::vec3& v3);
	QString float2QString(float f);

	bool isPropertyVisible(const QString& name);

	QWidget* drawIntField(uint index, const QString& name, int value);
	QWidget* drawFloatField(uint index, const QString& name, float value);
	QWidget* drawTextureField(uint index, const QString& name, Texture value);
	QWidget* drawVec3Field(uint index, const QString& name, const glm::vec3& value);
	QWidget* drawColor4Field(uint index, const QString& name, const glm::vec4& value);
	QWidget* drawVec4Field(uint index, const QString& name, const glm::vec4& value);

private:
	Sprite target_;
	QList<QWidget*> groups_;
};
