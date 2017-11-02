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

protected:
	// TODO: update hierarchy.
	virtual void timerEvent(QTimerEvent *event);
	virtual void keyPressEvent(QKeyEvent *event);
	virtual void OnEngineLogMessage(int level, const char* message);

private slots:
	void screenCapture();

private:
	void setupUI();

private:
	int timer_;
	Ui::Suede ui;
};
