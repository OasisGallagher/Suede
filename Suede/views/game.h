#pragma once

#include "window.h"

class QWidget;
class PostEffect;
class CameraController;

class Game : public Window {
	Q_OBJECT

public:
	static Game* get();

protected:
	virtual bool eventFilter(QObject* watched, QEvent* event);

private slots:
	void update();

private:
	Game();
	~Game();

	void wheelEvent(QWheelEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
	void keyPressEvent(QKeyEvent* event);
	void createScene();

private:
	virtual void initialize();

private:
	bool sceneCreated_;
	QWidget* canvas_;
	PostEffect* grayscale_;
	PostEffect* inversion_;
	CameraController* controller_;
};