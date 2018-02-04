#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "tools/math2.h"
#include "debug/debug.h"
#include "windows/hierarchy.h"
#include "cameracontroller.h"

CameraController::CameraController() : lpressed_(false), mpressed_(false), rpressed_(false) {
}

void CameraController::setCamera(Transform value) {
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
	glm::vec3 p = glm::vec3(pos.x() * width_ - 1.f, pos.y() * height_ - 1.f, 0);

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
		rotateAroundEntity(pos, lpos_);
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
	delta.setY(-delta.y());
	glm::quat qx = glm::angleAxis(Math::Radians(0.05f * delta.x()), camera_->GetUp());
	glm::quat qy = glm::angleAxis(Math::Radians(0.05f * delta.y()), camera_->GetRight());
	camera_->SetRotation(qx * qy * camera_->GetRotation());
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

extern uint roomEntityID;

#include <QDebug>
#include "world.h"

void CameraController::rotateAroundEntity(const QPoint& mousePos, QPoint& oldPos) {
	Entity selected = WorldInstance()->GetEntity(roomEntityID);

	if (!selected || selected->GetTransform()->GetPosition() == camera_->GetPosition()) {
		return;
	}

	if (oldPos != mousePos) {
#if ARC_BALL
		glm::vec3 va = calculateArcBallVector(oldPos);
		glm::vec3 vb = calculateArcBallVector(mousePos);

		glm::quat rot = glm::quat(glm::dot(va, vb), glm::cross(va, vb));
		rot = glm::pow(rot, 1 / 5.f);

		glm::vec3 dir = camera_->GetPosition() - selected->GetPosition();
		camera_->SetPosition(selected->GetPosition() + rot * dir);

		glm::vec3 forward = -normalize(selected->GetPosition() - camera_->GetPosition());
		glm::vec3 up = rot * camera_->GetUp();
		glm::vec3 right = glm::cross(up, forward);
		up = glm::cross(forward, right);

		glm::mat3 m3(right, up, forward);
		camera_->SetRotation(glm::normalize(glm::quat(m3)));
#else
		glm::vec3 bp(camera_->GetPosition() - selected->GetTransform()->GetPosition());
		QPoint delta = mousePos - oldPos;
		glm::quat qx = glm::angleAxis(0.05f * delta.x(), camera_->GetUp());
		glm::quat qy = glm::angleAxis(0.05f * delta.y(), camera_->GetRight());

		qx *= qy;
		
		bp = qx * bp + selected->GetTransform()->GetPosition();

		camera_->SetPosition(bp);

		glm::vec3 forward = -normalize(selected->GetTransform()->GetPosition() - camera_->GetPosition());
		glm::vec3 right = qx * camera_->GetRight();
		right.y = 0;
		Math::Orthogonalize(right, forward);

		glm::vec3 up = glm::cross(forward, right);
		
		glm::quat q(glm::mat3(right, up, forward));
		camera_->SetRotation(glm::normalize(q));
		
		//camera_->SetRotation(glm::normalize(qy * qx * camera_->GetRotation()));
#endif

		oldPos = mousePos;
	}
}
