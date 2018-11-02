#include "selectiongizmos.h"

SUEDE_DEFINE_COMPONENT(SelectionGizmos, GizmosPainter)

void SelectionGizmos::OnDrawGizmos() {
	Color oldColor = Gizmos::GetColor();
	Gizmos::SetColor(Color::green);

	for (GameObject go : selection_) {
		if (!go->GetActive()) {
			continue;
		}

		const Bounds& bounds = go->GetBounds();
		if (!bounds.IsEmpty()) {
			Gizmos::DrawWireCuboid(bounds.center, bounds.size);
		}
		else {
			Gizmos::DrawWireSphere(go->GetTransform()->GetPosition(), 5);
		}
	}

	Gizmos::SetColor(oldColor);
}
