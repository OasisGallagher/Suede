#pragma once
#include "camera.h"
#include <QMouseEvent>

class CameraController {
public:
	CameraController();

public:
	void setCamera(Camera value);

	void onMouseWheel(int delta);
	void onMouseMove(const QPoint& pos);

	void onMouseRelease(Qt::MouseButton button);
	void onMousePress(Qt::MouseButton button, const QPoint& pos);

private:
	void moveCamera(const QPoint& mousePos);
	void rotateCamera(const QPoint& mousePos);
	void rotateAroundSprite(const QPoint& mousePos);

private:
	Camera camera_;

	bool lpressed_;
	bool mpressed_;
	bool rpressed_;

	QPoint lpos_;
	QPoint mpos_;
	QPoint rpos_;
};
