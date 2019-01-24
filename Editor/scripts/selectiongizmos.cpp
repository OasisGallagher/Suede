#include "selectiongizmos.h"

#include "light.h"
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

		Light light = go->GetComponent<Light>();
		if (light && light->GetType() == LightType::Directional) {
			glm::vec3 pos = go->GetTransform()->GetPosition();
			Gizmos::DrawLines({ pos , pos + light->GetTransform()->GetForward() * 9.f });
		}
	}

	Gizmos::SetColor(oldColor);
}

#include "handles.h"

void SelectionGizmos::setSelection(const QList<GameObject>& value) {
	selection_.clear();
	selection_.reserve(value.size());

	for (const GameObject& go : value) {
		selection_.push_back(go.get());
		if (!go->GetComponent<Handles>()) {
			go->AddComponent<Handles>();
		}
	}
}
