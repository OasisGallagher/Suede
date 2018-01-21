#pragma once

#include <QDockWidget>
#include "childwindow.h"

class Canvas;
class ImageEffect;
class CameraController;

class Game : public QDockWidget, public ChildWindow {
	Q_OBJECT

public:
	static Game* get();

public:
	Game(QWidget* parent);
	~Game();

public:
	virtual void init();
	virtual void awake();

private slots:
	void update();

protected:
	virtual void wheelEvent(QWheelEvent* event);
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void resizeEvent(QResizeEvent* event);
	virtual void keyPressEvent(QKeyEvent* event);
	virtual void timerEvent(QTimerEvent *event);

private:
	void start();
	void createScene();

private:
	bool initialized_;
	int updateTimer_;

	Canvas* canvas_;
	ImageEffect* grayscale_;
	ImageEffect* inversion_;
	CameraController* controller_;
};
