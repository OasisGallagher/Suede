#pragma once

#include <QtWidgets/QMainWindow>
#include "engine.h"
#include "ui_suede.h"

class Canvas;
class Console;
class Hierarchy;

class Suede : public QMainWindow, public EngineLogReceiver {
	Q_OBJECT

public:
	Suede(QWidget *parent = 0);
	~Suede();

public:
	enum {
		ChildWindowGame = 1,
		ChildWindowConsole,
		ChildWindowInspector,
		ChildWindowHierarchy,

		ChildWindowCount,
	};

public:
	bool childWindowVisible(int index);
	void showChildWindow(int index, bool show);

protected:
	// TODO: update hierarchy.
	virtual void keyPressEvent(QKeyEvent *event);
	virtual void OnEngineLogMessage(int level, const char* message);

private slots:
	void screenCapture();

private:
	void setupUI();

private:
	Ui::Suede ui;
	QDockWidget* dockWidgets_[ChildWindowCount];
};
