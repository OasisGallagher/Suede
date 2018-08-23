#pragma once
#include <QLineEdit>
#include <QDockWidget>

#include <glm/glm.hpp>

#include "world.h"
#include "profiler.h"
#include "../winbase.h"

class CustomInspector;

class Inspector : public QDockWidget, public WinSingleton<Inspector> {
	Q_OBJECT

public:
	Inspector(QWidget* parent);
	~Inspector();

public:
	virtual void init(Ui::Suede* ui);
	virtual void awake();
	virtual void tick();


private slots:
	void onSelectionChanged(const QList<Entity>& selected, const QList<Entity>& deselected);

private:
	void onGui();
	void drawGui();

	void addInspector(CustomInspector* inspector);

	void drawBasics();
	void drawTags();
	void drawTransform();
	void drawComponents();

	void drawCamera();
private:
	Entity target_;
	char namebuffer_[256];

	struct ImFont* imguiFont_;
};
