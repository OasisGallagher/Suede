#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "engine.h"
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

void CameraController::onMouseMove(const QPoint& pos) {
	if (lpressed_) {
		rotateAroundSprite(pos);
	}

	if (mpressed_) {
		moveCamera(pos);
	}

	if (rpressed_) {
		rotateCamera(pos);
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

void CameraController::rotateCamera(const QPoint& mousePos) {
	QPoint delta = mousePos - rpos_;
	rpos_ = mousePos;
	glm::vec3 euler = camera_->GetEulerAngles();
	euler.x += 0.05f * delta.y();
	euler.y += 0.05f * delta.x();
	camera_->SetEulerAngles(euler);
}

void CameraController::moveCamera(const QPoint& mousePos) {
	QPoint delta = mousePos - mpos_;
	mpos_ = mousePos;
	glm::vec3 up = camera_->GetUp();
	glm::vec3 right = camera_->GetRight();

	up *= 0.05f * (float)delta.y();
	right *= 0.05f * (float)delta.x();

	camera_->SetPosition(camera_->GetPosition() + up + right);
}

// TODO: bug when set firstTime to 'true'.
bool firstTime = false;
void CameraController::rotateAroundSprite(const QPoint& mousePos) {
	Sprite selected = Hierarchy::get()->selectedSprite();
	if (!selected || selected->GetPosition() == camera_->GetPosition()) { return; }
	if (firstTime) {
		glm::vec3 dir;
		float theta = 0.134259f, phi = -1.07115f;
		dir.x = sinf(theta) * cosf(phi);
		dir.y = cosf(theta);
		dir.z = sinf(theta) *sin(phi);

		glm::vec3 pos = selected->GetPosition() + dir * 18.f;
		/*
		glm::quat q;
		q *= glm::angleAxis(delta.x() * 0.05f, glm::vec3(0, 1, 0));
		q *= glm::angleAxis(delta.y() * 0.05f, glm::vec3(1, 0, 0));

		glm::vec3 pos = q * dir + selected->GetPosition();
		*/
		camera_->SetPosition(pos);

		glm::quat q = glm::quat(glm::lookAt(camera_->GetPosition(), selected->GetPosition(), camera_->GetUp()));
		q = glm::conjugate(q);
		camera_->SetRotation(q);
		firstTime = false;
		return;
	}

	QPoint delta = mousePos - lpos_;
	lpos_ = mousePos;

	glm::vec3 dir = camera_->GetPosition() - selected->GetPosition();
#ifdef RAW
	glm::vec3 dir2 = glm::normalize(dir);

	float len = glm::length(dir);
	float theta = acosf(dir.y / len);
	float phi = atan2f(dir.z, dir.x);
	
	//theta += delta.y() * 0.005f;
	phi += 10 /*delta.x()*/ * 0.005f;
	
	dir.x = sinf(theta) * cosf(phi);
	dir.y = cosf(theta);
	dir.z = sinf(theta) *sinf(phi);

	Engine::get()->logger()->Log(std::string("theta ") + std::to_string(theta) + ", phi " + std::to_string(phi) + "(" + std::to_string(dir.x) + ", " + std::to_string(dir.y) + ", " + std::to_string(dir.z) + ")");

	glm::vec3 pos = selected->GetPosition() + dir * len;
#else
	glm::quat q;
	q *= glm::angleAxis(delta.y() * -0.005f, glm::vec3(1, 0, 0));
	q *= glm::angleAxis(delta.x() * -0.005f, glm::vec3(0, 1, 0));

	glm::vec3 pos = q * dir + selected->GetPosition();
#endif

	camera_->SetPosition(pos);

	glm::quat rq = glm::quat(glm::lookAt(camera_->GetPosition(), selected->GetPosition(), camera_->GetUp()));
	rq = glm::conjugate(rq);
	camera_->SetRotation(rq);
}
