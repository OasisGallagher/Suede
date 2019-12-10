#pragma once
#include "transform.h"
#include "behaviour.h"

#include <QMouseEvent>

class CameraController : public QObject, public IBehaviour {
	Q_OBJECT
	Q_PROPERTY(Vector3 MoveSpeed READ moveSpeed WRITE setMoveSpeed)
	Q_PROPERTY(Vector2 OrientSpeed READ orientSpeed WRITE setOrientSpeed)
	Q_PROPERTY(Vector2 RotateSpeed READ rotateSpeed WRITE setRotateSpeed)

	SUEDE_DECLARE_COMPONENT()

public:
	CameraController();
	~CameraController();

public:
	void setView(QWidget* value) { view_ = value; }

	Vector3 moveSpeed() { return moveSpeed_; }
	void setMoveSpeed(const Vector3& value) { moveSpeed_ = value; }

	Vector2 orientSpeed() { return orientSpeed_; }
	void setOrientSpeed(const Vector2& value) { orientSpeed_ = value; }

	Vector2 rotateSpeed() { return rotateSpeed_; }
	void setRotateSpeed(const Vector2& value) { rotateSpeed_ = value; }

public:
	void Awake() { camera_ = GetTransform(); }
	void Update();

private:
	void moveCamera(const Vector2& mousePos, Vector2& oldPos);
	void rotateCamera(const Vector2& mousePos, Vector2& oldPos);
	void rotateAroundGameObject(const Vector2& mousePos, Vector2& oldPos);
	Vector3 calculateArcBallVector(const Vector2& point);

private:
	Transform camera_;

	QWidget* view_;
	Vector2 pos_;
	bool moving_ = false;

	Vector3 moveSpeed_;
	Vector2 orientSpeed_;
	Vector2 rotateSpeed_;
};
