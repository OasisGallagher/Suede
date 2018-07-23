#pragma once

#include <QtWidgets/QMainWindow>

#include "ui_suede.h"

#include "debug/debug.h"
#include "tools/enum.h"
#include "os/filesystem.h"

BETTER_ENUM(ChildWindowType, int,
	Game,
	Console,
	Inspector,
	Hierarchy
);

class LightDialog;
class Suede : public QMainWindow, public LogReceiver {
	Q_OBJECT

public:
	Suede(QWidget *parent = 0);
	~Suede();

public:
	void awake();

public:
	bool childWindowVisible(int index);
	void showChildWindow(int index, bool show);

signals:
	void aboutToClose();

protected:
	virtual void OnLogMessage(LogLevel level, const char* message);

	virtual void closeEvent(QCloseEvent *event);
	virtual void keyPressEvent(QKeyEvent *event);

private slots:
	void aboutBox();
	void screenCapture();
	void onShowWindowsMenu();
	void onShowEnvironment();
	void onToggleWindowVisible();

private:
	void setupUI();

	void initializeHelpMenu();
	void initializeFileMenu();
	void initializeWindowsMenu();

private:
	Ui::Suede ui;
	LightDialog* envDialog_;
	QDockWidget** childWindows_;
};
