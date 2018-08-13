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
	Hierarchy,
	Lighting
)

class Suede : public QMainWindow, public LogReceiver {
	Q_OBJECT

public:
	Suede(QWidget *parent = 0);
	~Suede();

public:
	void init();
	void awake();
	void tick();

public:
	bool childWindowVisible(ChildWindowType index);
	void showChildWindow(ChildWindowType index, bool show);

signals:
	void aboutToClose();

protected:
	virtual void OnLogMessage(LogLevel level, const char* message);

	virtual void closeEvent(QCloseEvent *event);
	virtual void keyPressEvent(QKeyEvent *event);

private slots:
	void onAbout();
	void onPreferences();
	void onScreenCapture();
	void onShowWindowMenu();
	void onToggleWindowVisible();

private:
	void setupUI();

	void initializeLayout();
	void initializeHelpMenu();
	void initializeFileMenu();
	void initializeEditMenu();
	void initializeWindowMenu();

	void onErrorMessage(const char* message);

private:
	Ui::Suede ui;
	QDockWidget** childWindows_;
};
