#pragma once
#include <QtWidgets/QMainWindow>

#include "ui_editor.h"

#include "tools/enum.h"
#include "os/filesystem.h"
#include "childwindow.h"

class Selection;
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

	Selection* selection() { return selection_; }

signals:
	void aboutToClose();

protected:
	virtual void closeEvent(QCloseEvent *event);
	virtual void keyPressEvent(QKeyEvent *event);

private slots:
	void onAbout();
	void onTogglePlay();
	void onPreferences();
	void onScreenCapture();
	void onShowWindowMenu();
	void onToggleWindowVisible();

private:
	void setupUI();

	void initializeLayout();
	void initializeToolBar();
	void initializeHelpMenu();
	void initializeFileMenu();
	void initializeEditMenu();
	void initializeWindowMenu();

private:
	Ui::Editor ui_;
	bool playing_ = true;
	QAction* playAction_ = nullptr;

	Selection* selection_;

	Preferences* preferences_;
	ChildWindow** childWindows_;
};
