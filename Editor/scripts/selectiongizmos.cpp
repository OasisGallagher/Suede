#include "selectiongizmos.h"

#include "gizmos.h"

SUEDE_DEFINE_COMPONENT(SelectionGizmos, IGizmosPainter)

void SelectionGizmos::OnDrawGizmos() {
	if (selection_.empty()) { return; }

	Color oldColor = Gizmos::GetColor();

	for (IGameObject* ptr : selection_) {
		GameObject go(ptr);

		if (!go->GetActive()) {
			continue;
		}

		Gizmos::SetColor(Color::yellow);

		const Bounds& bounds = go->GetBounds();
		if (!bounds.IsEmpty()) {
			//body->ShowCollisionShape(true);
			Gizmos::DrawWireCuboid(bounds.center, bounds.size);
		}
		else {
			Gizmos::DrawWireSphere(go->GetTransform()->GetPosition(), 1);
		}

		glm::vec3 pos = go->GetTransform()->GetPosition();

		Gizmos::SetColor(Color::red);
		Gizmos::DrawLines({pos, pos + go->GetTransform()->GetRight() * 5.f});

		Gizmos::SetColor(Color::green);
		Gizmos::DrawLines({ pos, pos + go->GetTransform()->GetUp() * 5.f });

		Gizmos::SetColor(Color::blue);
		Gizmos::DrawLines({ pos, pos + go->GetTransform()->GetForward() * 5.f });
	}

	Gizmos::SetColor(oldColor);
}

void SelectionGizmos::setSelection(const QList<GameObject>& value) {
	selection_.clear();
	selection_.reserve(value.size());

	for (const GameObject& go : value) {
		selection_.push_back(go.get());
	}
}
