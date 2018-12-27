#include "selectiongizmos.h"

#include "gizmos.h"

SUEDE_DEFINE_COMPONENT(SelectionGizmos, IGizmosPainter)

void SelectionGizmos::OnDrawGizmos() {
	if (selection_.empty()) { return; }

	Color oldColor = Gizmos::GetColor();
	Gizmos::SetColor(Color::green);

	for (suede_weak_ref<GameObject> ref : selection_) {
		GameObject go = ref.lock();

		if (!go) {
			Debug::LogError("invalid weak reference");
			continue;
		}

		if (!go->GetActive()) {
			continue;
		}

		const Bounds& bounds = go->GetBounds();
		if (!bounds.IsEmpty()) {
			//body->ShowCollisionShape(true);
			Gizmos::DrawWireCuboid(bounds.center, bounds.size);
		}
		else {
			Gizmos::DrawWireSphere(go->GetTransform()->GetPosition(), 1);
		}

		Gizmos::DrawLines({ go->GetTransform()->GetPosition() ,go->GetTransform()->GetPosition() + go->GetTransform()->GetForward() * 10.f });
	}

	Gizmos::SetColor(oldColor);
}

void SelectionGizmos::setSelection(const QList<GameObject>& value) {
	selection_.clear();
	selection_.reserve(value.size());

	for (const GameObject& go : value) {
		selection_.push_back(go);
	}
}
