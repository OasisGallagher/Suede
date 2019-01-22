#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "input.h"
#include "tools/math2.h"
#include "debug/debug.h"
#include "cameracontroller.h"
#include "windows/hierarchy/hierarchy.h"

//#define ARC_BALL

SUEDE_DEFINE_COMPONENT(CameraController, IBehaviour)

CameraController::CameraController() 
	: orientSpeed_(0.005f, 0.005f), rotateSpeed_(0.02f, 0.02f)
	, moveSpeed_(-0.1f, 0.1f, 0.1f), moving_(false) {
}

void CameraController::Update() {
	if (Input::GetMouseWheelDelta() != 0) {
		glm::vec3 fwd = camera_->GetForward();
		camera_->SetPosition(camera_->GetPosition() + fwd * moveSpeed_.z * (float)Input::GetMouseWheelDelta());
	}

	if (moving_ != Input::GetMouseButton(1)) {
		moving_ = !moving_;
		view_->setCursor(moving_ ? Qt::ClosedHandCursor : Qt::ArrowCursor);
	}

	if (Input::GetMouseButtonDown(0) || Input::GetMouseButtonDown(1) || Input::GetMouseButtonDown(2)) {
		pos_ = Input::GetMousePosition();
	}

	if (Input::GetMouseButton(0)) {
		rotateAroundGameObject(Input::GetMousePosition(), pos_);
	}
	else if (Input::GetMouseButton(1)) {
		moveCamera(Input::GetMousePosition(), pos_);
	}
	else if (Input::GetMouseButton(2)) {
		rotateCamera(Input::GetMousePosition(), pos_);
	}
}

void CameraController::rotateCamera(const glm::ivec2& mousePos, glm::ivec2& oldPos) {
	glm::ivec2 delta = mousePos - oldPos;
	oldPos = mousePos;

	glm::vec2 polar = Math::EuclideanToPolar(-camera_->GetForward(), camera_->GetUp());
	polar += glm::vec2(delta.y, delta.x) * orientSpeed_;

	glm::vec3 dir = Math::PolarToEuclidean(polar);
	glm::vec3 right = glm::vec3(Math::Sin(polar.y), 0, -Math::Cos(polar.y));

	camera_->SetRotation(glm::lookRotation(dir, glm::cross(dir, right)));
}

void CameraController::moveCamera(const glm::ivec2& mousePos, glm::ivec2& oldPos) {
	glm::ivec2 delta = oldPos - mousePos;
	delta.x = -delta.x;

	oldPos = mousePos;
	glm::vec3 up = camera_->GetUp();
	glm::vec3 right = camera_->GetRight();

	up *= moveSpeed_.y * delta.y;
	right *= moveSpeed_.x * delta.x;

	camera_->SetPosition(camera_->GetPosition() + up + right);
}

void CameraController::rotateAroundGameObject(const glm::ivec2& mousePos, glm::ivec2& oldPos) {
	GameObject selected = Hierarchy::instance()->selectedGameObject();

	if (!selected || selected->GetTransform()->GetPosition() == camera_->GetPosition()) {
		return;
	}

	if (oldPos != mousePos) {
		glm::vec2 delta = mousePos - oldPos;
		Transform transform = selected->GetTransform();
		transform->SetRotation(
			glm::angleAxis(delta.y * rotateSpeed_.x, glm::vec3(1, 0, 0)) * glm::angleAxis(delta.x * rotateSpeed_.y, glm::vec3(0, 1, 0)) * transform->GetRotation()
		);
//#ifdef ARC_BALL
//		glm::vec3 va = calculateArcBallVector(oldPos);
//		glm::vec3 vb = calculateArcBallVector(mousePos);
//
//		glm::quat rot = glm::quat(glm::dot(va, vb), glm::cross(va, vb));
//		rot = glm::pow(rot, 1 / 5.f);
//
//		glm::vec3 dir = camera_->GetPosition() - selected->GetTransform()->GetPosition();
//		camera_->SetPosition(selected->GetTransform()->GetPosition() + rot * dir);
//
//		glm::vec3 forward = -normalize(selected->GetTransform()->GetPosition() - camera_->GetPosition());
//		glm::vec3 up = rot * camera_->GetUp();
//		glm::vec3 right = glm::cross(up, forward);
//		up = glm::cross(forward, right);
//
//		glm::mat3 m3(right, up, forward);
//		camera_->SetRotation(glm::normalize(glm::quat(m3)));
//#else
//		glm::vec3 bp(camera_->GetPosition() - selected->GetTransform()->GetPosition());
//
//		glm::ivec2 delta = mousePos - oldPos;
//		glm::quat qx = glm::angleAxis(rotateSpeed_.x * delta.x, camera_->GetUp());
//		glm::quat qy = glm::angleAxis(rotateSpeed_.y * delta.y, camera_->GetRight());
//
//		qx *= qy;
//		
//		bp = qx * bp + selected->GetTransform()->GetPosition();
//
//		camera_->SetPosition(bp);
//
//		glm::quat q(glm::lookAt(camera_->GetPosition(), selected->GetTransform()->GetPosition(), glm::vec3(0, 1, 0)));
//		camera_->SetRotation(glm::conjugate(q));
//
//		/*glm::vec3 forward = -glm::normalize(selected->GetTransform()->GetPosition() - camera_->GetPosition());
//		glm::vec3 right = qx * camera_->GetRight();
//		right.y = 0;
//		Math::Orthogonalize(right, forward);
//
//		glm::vec3 up = glm::cross(forward, right);
//
//		glm::quat q(glm::mat3(right, up, forward));
//		camera_->SetRotation(glm::normalize(q));*/
//#endif

		oldPos = mousePos;
	}
}
