#pragma once
#include "camera.h"
#include <QMouseEvent>

class CameraController {
public:
	CameraController();

public:
	void setCamera(Camera value);

	void onResize(const QSize& size);

	void onMouseWheel(int delta);
	void onMouseMove(const QPoint& pos);

	void onMouseRelease(Qt::MouseButton button);
	void onMousePress(Qt::MouseButton button, const QPoint& pos);

private:
	void moveCamera(const QPoint& mousePos);
	void rotateCamera(QPoint mousePos);
	void rotateAroundSprite(const QPoint& mousePos);
	glm::vec3 arcBallVector(const QPoint& point);

	glm::vec3 sphereCoords(float x, float y);

private:
	Camera camera_;

	bool lpressed_;
	bool mpressed_;
	bool rpressed_;

	float width_;
	float height_;

	QPoint lpos_;
	QPoint mpos_;
	QPoint rpos_;
};
