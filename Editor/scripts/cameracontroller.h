#pragma once
#include "transform.h"
#include "behaviour.h"

#include <QMouseEvent>

class CameraController : public QObject, public Behaviour {
	Q_OBJECT
	Q_PROPERTY(glm::vec3 MoveSpeed READ moveSpeed WRITE setMoveSpeed)
	Q_PROPERTY(glm::vec2 OrientSpeed READ orientSpeed WRITE setOrientSpeed)
	Q_PROPERTY(glm::vec2 RotateSpeed READ rotateSpeed WRITE setRotateSpeed)

	SUEDE_DECLARE_COMPONENT()

public:
	CameraController();

public:
	void setView(QWidget* view);

	glm::vec3 moveSpeed() { return moveSpeed_; }
	void setMoveSpeed(const glm::vec3& value) { moveSpeed_ = value; }

	glm::vec2 orientSpeed() { return orientSpeed_; }
	void setOrientSpeed(const glm::vec2& value) { orientSpeed_ = value; }

	glm::vec2 rotateSpeed() { return rotateSpeed_; }
	void setRotateSpeed(const glm::vec2& value) { rotateSpeed_ = value; }

public:
	void Awake() { camera_ = GetTransform(); }

public:
	void onMouseWheel(int delta);
	void onMouseMove(const QPoint& pos);

	void onMouseRelease(Qt::MouseButton button);
	void onMousePress(Qt::MouseButton button, const QPoint& pos);

private:
	void moveCamera(const QPoint& mousePos, QPoint& oldPos);
	void rotateCamera(const QPoint& mousePos, QPoint& oldPos);
	void rotateAroundGameObject(const QPoint& mousePos, QPoint& oldPos);
	glm::vec3 calculateArcBallVector(const QPoint& point);

private:
	Transform camera_;

	QPoint pos_;
	Qt::MouseButtons buttons_;

	QWidget* view_;

	glm::vec3 moveSpeed_;
	glm::vec2 orientSpeed_;
	glm::vec2 rotateSpeed_;
};
