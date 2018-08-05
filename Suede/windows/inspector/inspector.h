#pragma once
#include <QLineEdit>
#include <QDockWidget>

#include <glm/glm.hpp>

#include "world.h"
#include "profiler.h"
#include "../winbase.h"

class CustomInspector;

class Inspector : public QDockWidget, public WinSingleton<Inspector>, public WorldEventListener {
	Q_OBJECT

public:
	Inspector(QWidget* parent);
	~Inspector();

public:
	virtual void init(Ui::Suede* ui);
	virtual void awake();

public:
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
