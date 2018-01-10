#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "debug/debug.h"
#include "views/hierarchy.h"
#include "cameracontroller.h"

CameraController::CameraController() : lpressed_(false), mpressed_(false), rpressed_(false) {
}

void CameraController::setCamera(Camera value) {
	camera_ = value;
}

void CameraController::onMouseWheel(int delta) {
	glm::vec3 fwd = camera_->GetForward();
	camera_->SetPosition(camera_->GetPosition() + fwd * 0.005f * (float)delta);
}

void CameraController::onResize(const QSize& size) {
	width_ = 1.f / ((size.width() - 1.f) * 0.5f);
	height_ = 1.f / ((size.height() - 1.f) * 0.5f);
}

glm::vec3 CameraController::calculateArcBallVector(const QPoint& pos) {
	glm::vec3 p = glm::vec3(pos.x() * width_ - 1.f, 1 - pos.y() * height_, 0);

	float squared = glm::dot(p, p);
	if (squared <= 1 * 1) {
		p.z = sqrtf(1 * 1 - squared);
	}
	else {
		p = glm::normalize(p);
	}

	return p;
}

void CameraController::onMouseMove(const QPoint& pos) {
	if (lpressed_) {
		rotateAroundSprite(pos, lpos_);
	}

	if (mpressed_) {
		moveCamera(pos, mpos_);
	}

	if (rpressed_) {
		rotateCamera(pos, rpos_);
	}
}

void CameraController::onMouseRelease(Qt::MouseButton button) {
	if (button == Qt::MiddleButton) {
		mpressed_ = false;
	}
	else if (button == Qt::RightButton) {
		rpressed_ = false;
	}
	else if (button == Qt::LeftButton) {
		lpressed_ = false;
	}
}

void CameraController::onMousePress(Qt::MouseButton button, const QPoint & pos) {
	if (button == Qt::LeftButton) {
		lpos_ = pos;
		lpressed_ = true;
	}

	if (button == Qt::MiddleButton) {
		mpos_ = pos;
		mpressed_ = true;
	}

	if (button == Qt::RightButton) {
		rpos_ = pos;
		rpressed_ = true;
	}
}

void CameraController::rotateCamera(const QPoint& mousePos, QPoint& oldPos) {
	QPoint delta = mousePos - oldPos;
	oldPos = mousePos;
	glm::vec3 euler = camera_->GetEulerAngles();
	euler.x += 0.05f * delta.y();
	euler.y += 0.05f * delta.x();
	camera_->SetEulerAngles(euler);
}

void CameraController::moveCamera(const QPoint& mousePos, QPoint& oldPos) {
	QPoint delta = mousePos - oldPos;
	oldPos = mousePos;
	glm::vec3 up = camera_->GetUp();
	glm::vec3 right = camera_->GetRight();

	up *= 0.05f * (float)delta.y();
	right *= 0.05f * (float)delta.x();

	camera_->SetPosition(camera_->GetPosition() + up + right);
}

extern uint ballSpriteID;
extern uint roomSpriteID;

#include <QDebug>
#include "world.h"
#include "tools/math2.h"

void CameraController::rotateAroundSprite(const QPoint& mousePos, QPoint& oldPos) {
	Sprite selected = WorldInstance()->GetSprite(roomSpriteID);
	Sprite ball = camera_;// WorldInstance()->GetSprite(ballSpriteID);

	glm::vec3 position = selected ? selected->GetPosition() : glm::vec3(0);
	if (position == ball->GetPosition()) {
		Debug::LogError("invalid selection");
		return;
	}

	QPoint mutablePos = mousePos;
	//mutablePos.setY(oldPos.y());

	if (oldPos != mutablePos) {
		glm::vec3 va = calculateArcBallVector(oldPos);
		glm::vec3 vb = calculateArcBallVector(mutablePos);
		glm::vec3 vc = glm::cross(va, vb);

		glm::mat3 worldToObject = glm::mat3(selected->GetWorldToLocalMatrix());
		glm::vec3 axis = worldToObject * glm::cross(va, vb);

		glm::quat rot = glm::pow(glm::quat(glm::dot(va, vb), axis), 1 / 5.f);

		glm::vec4 d2 = selected->GetWorldToLocalMatrix() * glm::vec4(ball->GetPosition(), 1);
		glm::vec3 dir(d2);

		//qDebug() << "+" << ball->GetPosition().x;

		glm::vec3 offset = (rot * dir);
		offset = (glm::mat3)selected->GetLocalToWorldMatrix() * offset;
		ball->SetPosition(selected->GetPosition() + offset);

		glm::vec3 forward = -normalize(selected->GetPosition() - ball->GetPosition());
		glm::vec3 up = glm::vec3(0, 1, 0);// glm::cross(forward, vc);
		glm::vec3 right = glm::cross(up, forward);
		up = glm::cross(forward, right);

		glm::mat3 m3(right, up, forward);
		glm::quat q(m3);
		ball->SetRotation(glm::normalize(q));

		qDebug() << glm::length(ball->GetPosition() - selected->GetPosition());
		oldPos = mutablePos;
	}
}
