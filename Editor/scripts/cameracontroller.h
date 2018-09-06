#pragma once
#include "transform.h"
#include <QMouseEvent>

class CameraController {
public:
	CameraController(QWidget* view);

public:
	void setCamera(Transform value);

	void onResize(const QSize& size);

	void onMouseWheel(int delta);
	void onMouseMove(const QPoint& pos);

	void onMouseRelease(Qt::MouseButton button);
	void onMousePress(Qt::MouseButton button, const QPoint& pos);

private:
	void moveCamera(const QPoint& mousePos, QPoint& oldPos);
	void rotateCamera(const QPoint& mousePos, QPoint& oldPos);
	void rotateAroundEntity(const QPoint& mousePos, QPoint& oldPos);
	glm::vec3 calculateArcBallVector(const QPoint& point);

private:
	Transform camera_;

	bool lpressed_;
	bool mpressed_;
	bool rpressed_;

	float width_;
	float height_;

	QPoint lpos_;
	QPoint mpos_;
	QPoint rpos_;
	QWidget* view_;
};
