#include "selectiongizmos.h"

SUEDE_DEFINE_COMPONENT(SelectionGizmos, GizmosPainter)

void SelectionGizmos::OnDrawGizmos() {
	Color oldColor = Gizmos::instance()->GetColor();
	Gizmos::instance()->SetColor(Color::green);

	for (GameObject go : selection_) {
		if (!go->GetActive()) {
			continue;
		}

		const Bounds& bounds = go->GetBounds();
		if (!bounds.IsEmpty()) {
			Gizmos::instance()->DrawWireCuboid(bounds.center, bounds.size);
		}
		else {
			Gizmos::instance()->DrawWireSphere(go->GetTransform()->GetPosition(), 5);
		}
	}

	Gizmos::instance()->SetColor(oldColor);
}
