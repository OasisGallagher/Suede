#pragma once
#include <QDockWidget>
#include "childwindow.h"

class Inspector : public QDockWidget, public ChildWindow {
	Q_OBJECT

public:
	static Inspector* get();

public:
	Inspector(QWidget* parent);
	~Inspector();

public:
	virtual void ready();
};
