#include "selectiongizmos.h"

#include "gizmos.h"
#include "engine.h"
#include "camera.h"
#include "renderer.h"
#include "math/matrix4.h"
#include "main/selection.h"

SUEDE_DEFINE_COMPONENT(SelectionGizmos, GizmosPainter)

void SelectionGizmos::Awake() {
	gizmos_ = Engine::GetSubsystem<Gizmos>();
	camera_ = GetGameObject()->GetComponent<Camera>();
	if (camera_ == nullptr) {
		SetEnabled(false);
		Debug::LogError("Missing Camera component");
	}
}

void SelectionGizmos::OnDrawGizmos() {
	Color oldColor = gizmos_->GetColor();
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
			gizmos_->SetColor(Color::yellow);
			gizmos_->DrawWireCuboid(bounds.center, bounds.size);
		}

		Vector3 pos = go->GetTransform()->GetPosition();
		float length = Mathf::Max(bounds.size.z, Mathf::Max(bounds.size.x, bounds.size.y));
		length = Mathf::Max(1.f, length);

		Matrix4 oldMatrix = gizmos_->GetMatrix();

		float distance = Vector3::Distance(cameraPosition, pos);
		gizmos_->SetMatrix(Matrix4::Translate(pos) * Matrix4::Scale(Vector3(distance / 120.f)));
		pos = Vector3::zero;

		gizmos_->SetColor(Color::red);
		gizmos_->DrawArrow(pos, pos + go->GetTransform()->GetRight() * length);

		gizmos_->SetColor(Color::green);
		gizmos_->DrawArrow(pos, pos + go->GetTransform()->GetUp() * length);

		gizmos_->SetColor(Color::blue);
		gizmos_->DrawArrow(pos, pos + go->GetTransform()->GetForward() * length);

		gizmos_->SetMatrix(oldMatrix);
	}

	gizmos_->SetColor(oldColor);
}
