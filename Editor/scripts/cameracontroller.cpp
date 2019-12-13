#include "input.h"
#include "math/mathf.h"
#include "debug/debug.h"
#include "cameracontroller.h"
#include "windows/hierarchy/hierarchywindow.h"

#define ARC_BALL

SUEDE_DEFINE_COMPONENT(CameraController, Behaviour)

CameraController::CameraController() 
	: orientSpeed_(0.3f, 0.3f), rotateSpeed_(0.05f, 0.05f), moveSpeed_(-0.05f, 0.05f, 0.05f) {
}

CameraController::~CameraController() {
}

void CameraController::Update() {
	if (Input::GetMouseWheelDelta() != 0) {
		Vector3 fwd = camera_->GetForward();
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

Vector3 CameraController::calculateArcBallVector(const Vector2& pos) {
	Vector3 p = Vector3(pos.x * view_->width() - 1.f, pos.y * view_->height() - 1.f, 0);

	float squared = p.GetSqrMagnitude();
	if (squared <= 1 * 1) {
		p.z = sqrtf(1 * 1 - squared);
	}
	else {
		Vector3::Normalize(p);
	}

	return p;
}

void CameraController::rotateCamera(const Vector2& mousePos, Vector2& oldPos) {
	Vector2 delta = oldPos - mousePos;
	oldPos = mousePos;

	Vector3 eulerAngles = camera_->GetEulerAngles();
	eulerAngles.x += delta.y * orientSpeed_.x;
	eulerAngles.y += delta.x * orientSpeed_.y;
	eulerAngles.z = 0;
	camera_->SetEulerAngles(eulerAngles);
}

void CameraController::moveCamera(const Vector2& mousePos, Vector2& oldPos) {
	Vector2 delta = oldPos - mousePos;
	delta.x = -delta.x;

	oldPos = mousePos;
	Vector3 up = camera_->GetUp();
	Vector3 right = camera_->GetRight();

	up *= moveSpeed_.y * delta.y;
	right *= moveSpeed_.x * delta.x;

	camera_->SetPosition(camera_->GetPosition() + up + right);
}

void CameraController::rotateAroundGameObject(const Vector2& mousePos, Vector2& oldPos) {
	if (!selection_ || selection_->GetTransform()->GetPosition() == camera_->GetPosition()) {
		return;
	}

	if (oldPos != mousePos) {
#ifdef ARC_BALL
		Vector3 va = calculateArcBallVector(oldPos);
		Vector3 vb = calculateArcBallVector(mousePos);

		Quaternion rot(Vector3::Dot(va, vb), Vector3::Cross(va, vb));
		Quaternion::Pow(rot, 1 / 5.f);

		Vector3 dir = camera_->GetPosition() - selection_->GetTransform()->GetPosition();
		camera_->SetPosition(selection_->GetTransform()->GetPosition() + rot * dir);

		Vector3 forward = -(selection_->GetTransform()->GetPosition() - camera_->GetPosition()).GetNormalized();
		Vector3 up = rot * camera_->GetUp();
		Vector3 right = Vector3::Cross(up, forward);
		up = Vector3::Cross(forward, right);

		Matrix4 rotMatrix(
			Vector4(right.x, right.y, right.z, 0),
			Vector4(up.x, up.y, up.z, 0),
			Vector4(forward.x, forward.y, forward.z, 0),
			Vector4(0, 0, 0, 1)
		);

		camera_->SetRotation(Quaternion(rotMatrix).GetNormalized());
#else
		Vector3 bp(camera_->GetPosition() - selection_->GetTransform()->GetPosition());

		Vector2 delta = mousePos - oldPos;
		Quaternion qx(rotateSpeed_.x * delta.x, camera_->GetUp());
		Quaternion qy(rotateSpeed_.y * delta.y, camera_->GetRight());

		qx *= qy;

		bp = qx * bp + selection_->GetTransform()->GetPosition();

		camera_->SetPosition(bp);

		Quaternion q(Matrix4::lookAt(camera_->GetPosition(), selection_->GetTransform()->GetPosition(), Vector3(0, 1, 0)));
		camera_->SetRotation(q.getConjugated());
#endif

		oldPos = mousePos;
	}
}
