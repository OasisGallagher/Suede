#pragma once

#include <QtWidgets/QMainWindow>

#include "ui_editor.h"

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

class Preferences;
class Editor : public QMainWindow, public LogReceiver {
	Q_OBJECT

public:
	Editor(QWidget *parent = 0);
	~Editor();

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

private:
	Ui::Editor ui;
	Preferences* preferences_;
	QDockWidget** childWindows_;
};
