#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "debug.h"
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

glm::vec3 CameraController::arcBallVector(const QPoint& point) {
	//glm::vec3 P = glm::vec3(1.f * point.x() / windowSize_.width() * 2 - 1.0,
	//	1.f * point.y() / windowSize_.height() * 2 - 1.0,
	//	0);

	auto P = glm::vec3(point.x() * width_ - 1.f, 1 - point.y() * height_, 0);

	P.y = -P.y;

	float OP_squared = P.x * P.x + P.y * P.y;
	if (OP_squared <= 1 * 1)
		P.z = sqrtf(1 * 1 - OP_squared);  // Pythagore
	else
		P = glm::normalize(P);  // nearest point
	return P;
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

std::string toString(const glm::vec3& v) {
	return std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z);
}

glm::vec3 CameraController::sphereCoords(float x, float y) {
	glm::vec3 pos = camera_->ScreenToWorldPoint(glm::vec3(x, y, 0));
	glm::vec3 m = pos - camera_->GetPosition();
	float radius = 5.f;

	float t = sqrtf(radius * radius / glm::dot(m, m));

	m = (camera_->GetPosition() + (m * t));
	m = glm::normalize(m);

	return m;
}

void CameraController::rotateCamera(QPoint mousePos) {
	//if (!Hierarchy::get()->selectedSprite()) {
	//	Debug::LogWarning("select a sprite first.");
	//	return;
	//}

	glm::vec3 sc0 = sphereCoords(rpos_.x(), rpos_.y());
	glm::vec3 sc1 = sphereCoords(mousePos.x(), mousePos.y());
	if (sc0 == sc1) {
		Debug::LogWarning("nothing rotated");
		return;
	}

	float cos2a = glm::dot(sc0, sc1);
	float sina = sqrtf((1.f - cos2a) * 0.5f);
	float cosa = sqrtf((1.f + cos2a) * 0.5f);
	glm::vec3 cross = glm::normalize(glm::cross(sc0, sc1)) * sina;
	glm::quat q(cosa, cross);

	glm::quat q2(glm::dot(sc0, sc1), glm::cross(sc0, sc1));

	Sprite sprite = Hierarchy::get()->selectedSprite();
	camera_->SetRotation(camera_->GetRotation() * q2);

	rpos_ = mousePos;
#if 0
// 	QPoint delta = mousePos - rpos_;
// 	rpos_ = mousePos;
// 	glm::vec3 euler = camera_->GetEulerAngles();
// 	euler.x += 0.05f * delta.y();
// 	euler.y += 0.05f * delta.x();
// 	camera_->SetEulerAngles(euler);
	if (rpos_ != mousePos) {
		//mousePos.setY(rpos_.y());
		glm::vec3 va = arcBallVector(rpos_);
		glm::vec3 vb = arcBallVector(mousePos);
		
		Engine::logger()->Log(toString(va) + ", " + toString(vb));

		float angle = acosf(glm::min(1.0f, glm::dot(va, vb)));
		float sa = sinf(angle);

		glm::vec3 axis_in_camera_coord = glm::cross(va, vb);
		
		//axis_in_camera_coord /= sa;

		Engine::logger()->Log(toString(axis_in_camera_coord));

		angle /= 5.f;

		float sd = glm::dot(axis_in_camera_coord, axis_in_camera_coord);

		glm::quat q;
		q.x = axis_in_camera_coord.x;// *sinf(angle);
		q.y = axis_in_camera_coord.y;// *sinf(angle);
		q.z = axis_in_camera_coord.z;// *sinf(angle);
		q.w = glm::dot(va, vb);//cosf(angle);

		//glm::mat3 camera2object = glm::inverse(glm::mat3(camera_->GetWorldToLocalMatrix()) * glm::mat3(mesh.object2world));
		//glm::vec3 axis_in_object_coord = camera2object * axis_in_camera_coord;
		//mesh.object2world = glm::rotate(mesh.object2world, glm::degrees(angle), axis_in_object_coord);
		glm::mat4 mat = glm::rotate(camera_->GetLocalToWorldMatrix(), angle, camera_->GetUp()/*axis_in_object_coord*/);

		glm::vec3 skew;
		glm::vec4 perspective;
		glm::vec3 translation;
		glm::quat rotation;
		glm::vec3 scale;
		//rotation = glm::conjugate(rotation);
		glm::decompose(mat, scale, rotation, translation, skew, perspective);
		//camera_->SetPosition(translation);
		camera_->SetRotation(camera_->GetRotation() * q);
		rpos_ = mousePos;
	}
#endif
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
static bool firstTime = false;
void CameraController::rotateAroundSprite(const QPoint& mousePos) {
	Sprite selected = Hierarchy::get()->selectedSprite();
	glm::vec3 position = selected ? selected->GetPosition() : glm::vec3(0);
	if (position == camera_->GetPosition()) {
		return;
	}

	if (firstTime) {
		glm::vec3 dir;
		float theta = 0.134259f, phi = -1.07115f;
		dir.x = sinf(theta) * cosf(phi);
		dir.y = cosf(theta);
		dir.z = sinf(theta) *sin(phi);

		glm::vec3 pos = position + dir * 18.f;
		/*
		glm::quat q;
		q *= glm::angleAxis(delta.x() * 0.05f, glm::vec3(0, 1, 0));
		q *= glm::angleAxis(delta.y() * 0.05f, glm::vec3(1, 0, 0));

		glm::vec3 pos = q * dir + position;
		*/
		camera_->SetPosition(pos);

		glm::quat q = glm::quat(glm::lookAt(camera_->GetPosition(), position, camera_->GetUp()));
		q = glm::conjugate(q);
		camera_->SetRotation(q);
		firstTime = false;
		return;
	}

	QPoint delta = mousePos - lpos_;
	lpos_ = mousePos;

	glm::vec3 dir = camera_->GetPosition() - position;
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

	Engine::logger()->Log(std::string("theta ") + std::to_string(theta) + ", phi " + std::to_string(phi) + "(" + std::to_string(dir.x) + ", " + std::to_string(dir.y) + ", " + std::to_string(dir.z) + ")");

	glm::vec3 pos = position + dir * len;
#else
	glm::quat q;
	q *= glm::angleAxis(delta.y() * -0.005f, glm::vec3(1, 0, 0));
	q *= glm::angleAxis(delta.x() * -0.005f, glm::vec3(0, 1, 0));

	glm::vec3 pos = q * dir + position;
#endif

	camera_->SetPosition(pos);

	glm::quat rq = glm::quat(glm::lookAt(camera_->GetPosition(), position, camera_->GetUp()));
	rq = glm::conjugate(rq);
	camera_->SetRotation(rq);
}
