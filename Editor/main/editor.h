#pragma once

#include <QTextStream>
#include <QtWidgets/QMainWindow>

#include "ui_editor.h"

#include "debug/debug.h"
#include "tools/enum.h"
#include "os/filesystem.h"
#include "childwindow.h"

class Preferences;
class Editor : public QMainWindow {
	Q_OBJECT

public:
	Editor(QWidget *parent = 0);
	~Editor();

public:
	void awake();
	void tick();

	Ui::Editor* ui() { return &ui_; }

public:
	template <class T>
	T* childWindow() { return dynamic_cast<T*>(childWindows_[T::WindowType]); }

signals:
	void aboutToClose();

protected:
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
	void onLogMessage(LogLevel level, const char* message);
	void writeLog(ConsoleMessageType type, const char* message);

	void initializeLayout();
	void initializeHelpMenu();
	void initializeFileMenu();
	void initializeEditMenu();
	void initializeWindowMenu();

private:
	Ui::Editor ui_;

	bool flush_;
	QFile logFile_;
	QTextStream logStream_;

	Preferences* preferences_;
	ChildWindow** childWindows_;
};
