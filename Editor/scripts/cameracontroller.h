#pragma once
#include "transform.h"
#include <QMouseEvent>

class CameraController {
public:
	CameraController(QWidget* view);

public:
	void setCamera(Transform value);

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

	QPoint pos_;
	Qt::MouseButtons buttons_;

	QWidget* view_;

	glm::vec2 orientSpeed_;
	glm::vec2 rotateSpeed_;
	glm::vec3 moveSpeed_;
};
