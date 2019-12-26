#include "selectiongizmos.h"

#include "engine.h"
#include "gizmos.h"

SUEDE_DEFINE_COMPONENT(SelectionGizmos, GizmosPainter)

void SelectionGizmos::OnDrawGizmos() {
	if (selection_.empty()) { return; }
	
	Gizmos* gizmos = Engine::GetSubsystem<Gizmos>();
	Color oldColor = gizmos->GetColor();

	for (GameObject* go : selection_) {
		if (!go->GetActive()) {
			continue;
		}

		gizmos->SetColor(Color::yellow);

		const Bounds& bounds = go->GetBounds();
		if (!bounds.IsEmpty()) {
			//body->ShowCollisionShape(true);
			gizmos->DrawWireCuboid(bounds.center, bounds.size);
		}
		else {
			gizmos->DrawWireSphere(go->GetTransform()->GetPosition(), 1);
		}

		Vector3 pos = go->GetTransform()->GetPosition();

		gizmos->SetColor(Color::red);
		gizmos->DrawLines({pos, pos + go->GetTransform()->GetRight() * 5.f});

		gizmos->SetColor(Color::green);
		gizmos->DrawLines({ pos, pos + go->GetTransform()->GetUp() * 5.f });

		gizmos->SetColor(Color::blue);
		gizmos->DrawLines({ pos, pos + go->GetTransform()->GetForward() * 5.f });
	}

	gizmos->SetColor(oldColor);
}

void SelectionGizmos::setSelection(const QList<GameObject*>& value) {
	selection_.clear();
	selection_.reserve(value.size());

	for (GameObject* go : value) {
		selection_.push_back(go);
	}
}
