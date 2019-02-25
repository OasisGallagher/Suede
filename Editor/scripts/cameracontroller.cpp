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

	if (Input::GetMouseButton(1)) {
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

