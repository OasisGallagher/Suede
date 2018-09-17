#include "gui.h"
#include "rect.h"
#include "transform.h"
#include "custominspector.h"

class TransformInspector : public CustomInspector {
public:
	virtual void onGui(Component component) {
		Transform transform = suede_dynamic_cast<Transform>(component);
		glm::vec3 v3 = transform->GetLocalPosition();
		if (GUI::Float3Field("P", v3)) {
			transform->SetPosition(v3);
		}

		v3 = transform->GetLocalEulerAngles();
		if (GUI::Float3Field("R", v3)) {
			transform->SetLocalEulerAngles(v3);
		}

		v3 = transform->GetLocalScale();
		if (GUI::Float3Field("S", v3)) {
			transform->SetLocalScale(v3);
		}
	}
};
