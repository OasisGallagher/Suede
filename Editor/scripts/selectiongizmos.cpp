#include "selectiongizmos.h"

#include "input.h"
#include "rigidbody.h"

SUEDE_DEFINE_COMPONENT(SelectionGizmos, GizmosPainter)

#include <windows.h>
#include "tools/string.h"

void SelectionGizmos::OnDrawGizmos() {
	if (selection_.empty()) { return; }

	Color oldColor = Gizmos::GetColor();
	Gizmos::SetColor(Color::green);

	for (GameObject go : selection_) {
		if (!go->GetActive()) {
			continue;
		}

		Rigidbody body = go->GetComponent<IRigidbody>();
		if (!body) { continue; }

		const Bounds& bounds = body->GetBounds();
		if (!bounds.IsEmpty()) {
			//body->ShowCollisionShape(true);
			Gizmos::DrawWireCuboid(bounds.center, bounds.size);
		}
		else {
			Gizmos::DrawWireSphere(go->GetTransform()->GetPosition(), 1);
		}
	}

	Gizmos::SetColor(oldColor);
}
