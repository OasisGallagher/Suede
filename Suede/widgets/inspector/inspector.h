#pragma once
#include <QLineEdit>
#include <QDockWidget>

#include <glm/glm.hpp>

#include "world.h"
#include "../base/childwindow.h"

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

	void onPositionChanged(const glm::vec3& value);
	void onRotationChanged(const glm::vec3& value);
	void onScaleChanged(const glm::vec3& value);

	void onSelectionChanged(const QList<Entity>& selected, const QList<Entity>& deselected);
	void onResetButtonClicked();

private:
	void showView(bool show);

	void addInspector(CustomInspector* inspector);
	void destroyInspectors();

	void redraw();

	void drawTags();
	void drawTransform();
	void drawInspectors();

	void onEntityTransformChanged(Entity target, uint prs);

private:
	Entity target_;
	QList<CustomInspector*> inspectors_;
};
