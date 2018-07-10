#pragma once
#include <QLineEdit>
#include <QDockWidget>

#include <glm/glm.hpp>

#include "world.h"
#include "windows/controls/childwindow.h"

class CustomInspector;

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

	void onTagChanged(int index);
	void onActiveChanged(int state);
	void onSelectionChanged(const QList<Entity>& selected, const QList<Entity>& deselected);
	void onTransformChanged();
	void onResetButtonClicked();

private:
	void initTransformUI();

	void showView(bool show);

	void addInspector(CustomInspector* inspector);
	void destroyInspectors();

	void redraw();

	void drawTags();
	void drawTransform();
	void drawInspectors();

	void onEntityTransformChanged(Entity target, uint prs);

	glm::vec3 readTransformFields(QLineEdit* x, QLineEdit* y, QLineEdit* z);
	void drawTransformFields(QLineEdit* x, QLineEdit* y, QLineEdit* z, const glm::vec3& v3);

private:
	Entity target_;
	QList<CustomInspector*> inspectors_;
};
