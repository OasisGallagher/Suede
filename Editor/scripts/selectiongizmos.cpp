#include "selectiongizmos.h"

#include "gizmos.h"
#include "engine.h"
#include "renderer.h"
#include "main/selection.h"

SUEDE_DEFINE_COMPONENT(SelectionGizmos, GizmosPainter)

void SelectionGizmos::Awake() {
	gizmos_ = Engine::GetSubsystem<Gizmos>();
}

void SelectionGizmos::OnDrawGizmos() {
	Color oldColor = gizmos_->GetColor();

	for (GameObject* go : selection_->gameObjects()) {
		if (!go->GetActive()) {
			continue;
		}

		gizmos_->SetColor(Color::yellow);

		Bounds bounds;
		for (Renderer* renderer : go->GetComponentsInChildren<Renderer>()) {
			bounds.Encapsulate(renderer->GetBounds());
		}

		if (!bounds.IsEmpty()) {
			//body->ShowCollisionShape(true);
			gizmos_->DrawWireCuboid(bounds.center, bounds.size);
		}
		else {
			gizmos_->DrawWireSphere(go->GetTransform()->GetPosition(), 1);
		}

		Vector3 pos = go->GetTransform()->GetPosition();

		gizmos_->SetColor(Color::red);
		gizmos_->DrawLines({pos, pos + go->GetTransform()->GetRight() * bounds.size.x});

		gizmos_->SetColor(Color::green);
		gizmos_->DrawLines({ pos, pos + go->GetTransform()->GetUp() * bounds.size.y });

		gizmos_->SetColor(Color::blue);
		gizmos_->DrawLines({ pos, pos + go->GetTransform()->GetForward() * bounds.size.z });
	}

	gizmos_->SetColor(oldColor);
}
