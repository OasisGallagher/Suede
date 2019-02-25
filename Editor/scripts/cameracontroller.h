#pragma once
#include "transform.h"
#include "behaviour.h"

#include <QMouseEvent>

class CameraController : public QObject, public IBehaviour {
	Q_OBJECT
	Q_PROPERTY(glm::vec3 MoveSpeed READ moveSpeed WRITE setMoveSpeed)
	Q_PROPERTY(glm::vec2 OrientSpeed READ orientSpeed WRITE setOrientSpeed)
	Q_PROPERTY(glm::vec2 RotateSpeed READ rotateSpeed WRITE setRotateSpeed)

	SUEDE_DECLARE_COMPONENT()

public:
	CameraController();
	~CameraController() {}

public:
	void setView(QWidget* value) { view_ = value; }

	glm::vec3 moveSpeed() { return moveSpeed_; }
	void setMoveSpeed(const glm::vec3& value) { moveSpeed_ = value; }

	glm::vec2 orientSpeed() { return orientSpeed_; }
	void setOrientSpeed(const glm::vec2& value) { orientSpeed_ = value; }

	glm::vec2 rotateSpeed() { return rotateSpeed_; }
	void setRotateSpeed(const glm::vec2& value) { rotateSpeed_ = value; }

public:
	virtual void Awake() { camera_ = GetTransform(); }
	virtual void Update();

private:
	void moveCamera(const glm::ivec2& mousePos, glm::ivec2& oldPos);
	void rotateCamera(const glm::ivec2& mousePos, glm::ivec2& oldPos);

private:
	Transform camera_;

	bool moving_;
	QWidget* view_;
	glm::ivec2 pos_;

	glm::vec3 moveSpeed_;
	glm::vec2 orientSpeed_;
	glm::vec2 rotateSpeed_;
};
