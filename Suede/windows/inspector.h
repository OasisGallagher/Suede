#pragma once
#include <QDockWidget>

#include <glm/glm.hpp>

#include "world.h"
#include "sprite.h"
#include "childwindow.h"

class QLineEdit;
class QPushButton;

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

	void onSpriteTransformChanged(SpriteTransformChangedEvent* e);

	glm::vec3 readFields(int i);
	void writeFields(int i, const glm::vec3& v3);
	QString float2QString(float f);

private:
	Sprite target_;

	QPushButton* reset_[3];
	QLineEdit* transform_[9];
};
