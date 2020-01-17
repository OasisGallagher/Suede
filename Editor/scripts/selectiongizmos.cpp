#include "selectiongizmos.h"

#include "gizmos.h"
#include "engine.h"
#include "camera.h"
#include "renderer.h"
#include "math/matrix4.h"
#include "main/selection.h"

SUEDE_DEFINE_COMPONENT(SelectionGizmos, GizmosPainter)

void SelectionGizmos::Awake() {
	selectionGizmos_ = Engine::GetSubsystem<Gizmos>();
	camera_ = GetGameObject()->GetComponent<Camera>();
	if (camera_ == nullptr) {
		SetEnabled(false);
		Debug::LogError("Missing Camera component");
	}
}

void SelectionGizmos::OnDrawGizmos() {
	Color oldColor = selectionGizmos_->GetColor();
	Vector3 cameraPosition = GetTransform()->GetPosition();

	for (GameObject* go : selection_->gameObjects()) {
		if (!go->GetActive()) {
			continue;
		}

		if (go == GetGameObject()) {
			continue;
		}

		Bounds bounds;
		for (Renderer* renderer : go->GetComponentsInChildren<Renderer>()) {
			bounds.Encapsulate(renderer->GetBounds());
		}

		if (!bounds.IsEmpty()) {
			selectionGizmos_->SetColor(Color::yellow);
			selectionGizmos_->DrawWireCuboid(bounds.center, bounds.size);
		}

		Vector3 pos = go->GetTransform()->GetPosition();
		float length = Mathf::Max(bounds.size.z, Mathf::Max(bounds.size.x, bounds.size.y));
		length = Mathf::Max(1.f, length);

		Matrix4 oldMatrix = selectionGizmos_->GetMatrix();

		float distance = Vector3::Distance(cameraPosition, pos);
		selectionGizmos_->SetMatrix(Matrix4::Translate(pos) * Matrix4::Scale(Vector3::one));// (distance / 120.f)));

		selectionGizmos_->SetColor(Color::red);
		selectionGizmos_->DrawArrow(Vector3::zero, go->GetTransform()->GetRight() * length);

		selectionGizmos_->SetColor(Color::green);
		selectionGizmos_->DrawArrow(Vector3::zero, go->GetTransform()->GetUp() * length);

		selectionGizmos_->SetColor(Color::blue);
		selectionGizmos_->DrawArrow(Vector3::zero, go->GetTransform()->GetForward() * length);

		selectionGizmos_->SetMatrix(oldMatrix);
	}

	selectionGizmos_->SetColor(oldColor);
}
