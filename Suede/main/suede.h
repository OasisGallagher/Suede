#pragma once

#include <QtWidgets/QMainWindow>

#include "ui_suede.h"

#include "os/os.h"
#include "debug/debug.h"

class Suede : public QMainWindow, public LogReceiver, public PromptCallback {
	Q_OBJECT

public:
	Suede(QWidget *parent = 0);
	~Suede();

public:
	void awake();

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
	virtual bool OnPrompt(const char* message);
	virtual void OnLogMessage(LogLevel level, const char* message);

	virtual void closeEvent(QCloseEvent *event);
	virtual void keyPressEvent(QKeyEvent *event);

private slots:
	void screenCapture();

private:
	void setupUI();

private:
	Ui::Suede ui;
	QDockWidget* childWindows_[ChildWindowCount];
};
