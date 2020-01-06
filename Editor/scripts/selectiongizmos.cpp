#include "selectiongizmos.h"

#include "gizmos.h"
#include "engine.h"
#include "renderer.h"

SUEDE_DEFINE_COMPONENT(SelectionGizmos, GizmosPainter)

void SelectionGizmos::Awake() {
	gizmos_ = Engine::GetSubsystem<Gizmos>();
}

void SelectionGizmos::OnDrawGizmos() {
	if (selection_.empty()) { return; }
	
	Color oldColor = gizmos_->GetColor();

	for (GameObject* go : selection_) {
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

void SelectionGizmos::setSelection(const QList<GameObject*>& value) {
	selection_.clear();

	for (GameObject* go : value) {
		selection_.push_back(go);
	}
}
